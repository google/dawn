// Copyright 2021 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <set>
#include <sstream>
#include <string>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/utils/TerribleCommandBuffer.h"
#include "dawn/utils/WireHelper.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

namespace utils {

namespace {

class WireServerTraceLayer : public dawn::wire::CommandHandler {
  public:
    WireServerTraceLayer(const char* dir, dawn::wire::CommandHandler* handler)
        : dawn::wire::CommandHandler(), mDir(dir), mHandler(handler) {
        const char* sep = GetPathSeparator();
        if (mDir.size() > 0 && mDir.back() != *sep) {
            mDir += sep;
        }
    }

    void BeginWireTrace(const char* name) {
        std::string filename = name;
        // Replace slashes in gtest names with underscores so everything is in one
        // directory.
        std::replace(filename.begin(), filename.end(), '/', '_');
        std::replace(filename.begin(), filename.end(), '\\', '_');

        // Prepend the filename with the directory.
        filename = mDir + filename;

        ASSERT(!mFile.is_open());
        mFile.open(filename, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

        // Write the initial 8 bytes. This means the fuzzer should never inject an
        // error.
        const uint64_t injectedErrorIndex = 0xFFFF'FFFF'FFFF'FFFF;
        mFile.write(reinterpret_cast<const char*>(&injectedErrorIndex), sizeof(injectedErrorIndex));
    }

    const volatile char* HandleCommands(const volatile char* commands, size_t size) override {
        if (mFile.is_open()) {
            mFile.write(const_cast<const char*>(commands), size);
        }
        return mHandler->HandleCommands(commands, size);
    }

  private:
    std::string mDir;
    dawn::wire::CommandHandler* mHandler;
    std::ofstream mFile;
};

class WireHelperDirect : public WireHelper {
  public:
    explicit WireHelperDirect(const DawnProcTable& procs) { dawnProcSetProcs(&procs); }

    wgpu::Instance RegisterInstance(WGPUInstance backendInstance) override {
        ASSERT(backendInstance != nullptr);
        return wgpu::Instance(backendInstance);
    }

    void BeginWireTrace(const char* name) override {}

    bool FlushClient() override { return true; }

    bool FlushServer() override { return true; }
};

class WireHelperProxy : public WireHelper {
  public:
    explicit WireHelperProxy(const char* wireTraceDir, const DawnProcTable& procs) {
        mC2sBuf = std::make_unique<utils::TerribleCommandBuffer>();
        mS2cBuf = std::make_unique<utils::TerribleCommandBuffer>();

        dawn::wire::WireServerDescriptor serverDesc = {};
        serverDesc.procs = &procs;
        serverDesc.serializer = mS2cBuf.get();

        mWireServer.reset(new dawn::wire::WireServer(serverDesc));
        mC2sBuf->SetHandler(mWireServer.get());

        if (wireTraceDir != nullptr && strlen(wireTraceDir) > 0) {
            mWireServerTraceLayer.reset(new WireServerTraceLayer(wireTraceDir, mWireServer.get()));
            mC2sBuf->SetHandler(mWireServerTraceLayer.get());
        }

        dawn::wire::WireClientDescriptor clientDesc = {};
        clientDesc.serializer = mC2sBuf.get();

        mWireClient.reset(new dawn::wire::WireClient(clientDesc));
        mS2cBuf->SetHandler(mWireClient.get());
        dawnProcSetProcs(&dawn::wire::client::GetProcs());
    }

    wgpu::Instance RegisterInstance(WGPUInstance backendInstance) override {
        ASSERT(backendInstance != nullptr);

        auto reservation = mWireClient->ReserveInstance();
        mWireServer->InjectInstance(backendInstance, reservation.id, reservation.generation);

        return wgpu::Instance::Acquire(reservation.instance);
    }

    void BeginWireTrace(const char* name) override {
        if (mWireServerTraceLayer) {
            return mWireServerTraceLayer->BeginWireTrace(name);
        }
    }

    bool FlushClient() override { return mC2sBuf->Flush(); }

    bool FlushServer() override { return mS2cBuf->Flush(); }

  private:
    std::unique_ptr<utils::TerribleCommandBuffer> mC2sBuf;
    std::unique_ptr<utils::TerribleCommandBuffer> mS2cBuf;
    std::unique_ptr<WireServerTraceLayer> mWireServerTraceLayer;
    std::unique_ptr<dawn::wire::WireServer> mWireServer;
    std::unique_ptr<dawn::wire::WireClient> mWireClient;
};

}  // anonymous namespace

std::unique_ptr<WireHelper> CreateWireHelper(const DawnProcTable& procs,
                                             bool useWire,
                                             const char* wireTraceDir) {
    if (useWire) {
        return std::unique_ptr<WireHelper>(new WireHelperProxy(wireTraceDir, procs));
    } else {
        return std::unique_ptr<WireHelper>(new WireHelperDirect(procs));
    }
}

WireHelper::~WireHelper() {
    dawnProcSetProcs(nullptr);
}

}  // namespace utils
