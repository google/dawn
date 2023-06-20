// Copyright 2023 The Dawn Authors
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

#include <fstream>
#include <memory>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMConstants_autogen.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMFuzzer.h"
#include "dawn/fuzzers/lpmfuzz/DawnLPMSerializer_autogen.h"
#include "dawn/fuzzers/lpmfuzz/dawn_lpm_autogen.pb.h"
#include "dawn/native/DawnNative.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/utils/TerribleCommandBuffer.h"
#include "dawn/webgpu_cpp.h"
#include "dawn/wire/ChunkedCommandSerializer.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireResult.h"
#include "dawn/wire/WireServer.h"
#include "testing/libfuzzer/libfuzzer_exports.h"

namespace {

class DevNull : public dawn::wire::CommandSerializer {
  public:
    size_t GetMaximumAllocationSize() const override {
        // Some fuzzer bots have a 2GB allocation limit. Pick a value reasonably below that.
        return 1024 * 1024 * 1024;
    }
    void* GetCmdSpace(size_t size) override {
        if (size > buf.size()) {
            buf.resize(size);
        }
        return buf.data();
    }
    bool Flush() override { return true; }

  private:
    std::vector<char> buf;
};

std::unique_ptr<dawn::native::Instance> sInstance;
static bool (*sAdapterSupported)(const dawn::native::Adapter&) = nullptr;

}  // namespace

namespace DawnLPMFuzzer {

int Initialize(int* argc, char*** argv) {
    // TODO(crbug.com/1038952): The Instance must be static because destructing the vkInstance with
    // Swiftshader crashes libFuzzer. When this is fixed, move this into Run so that error injection
    // for physical device discovery can be fuzzed.
    sInstance = std::make_unique<dawn::native::Instance>();
    sInstance->DiscoverDefaultPhysicalDevices();

    return 0;
}

int Run(const fuzzing::Program& program, bool (*AdapterSupported)(const dawn::native::Adapter&)) {
    sAdapterSupported = AdapterSupported;

    DawnProcTable procs = dawn::native::GetProcs();

    // Override requestAdapter to find an adapter that the fuzzer supports.
    procs.instanceRequestAdapter = [](WGPUInstance cInstance,
                                      const WGPURequestAdapterOptions* options,
                                      WGPURequestAdapterCallback callback, void* userdata) {
        std::vector<dawn::native::Adapter> adapters = sInstance->EnumerateAdapters();
        for (dawn::native::Adapter adapter : adapters) {
            if (sAdapterSupported(adapter)) {
                WGPUAdapter cAdapter = adapter.Get();
                dawn::native::GetProcs().adapterReference(cAdapter);
                callback(WGPURequestAdapterStatus_Success, cAdapter, nullptr, userdata);
                return;
            }
        }
        callback(WGPURequestAdapterStatus_Unavailable, nullptr, "No supported adapter.", userdata);
    };

    dawnProcSetProcs(&procs);

    DevNull devNull;
    dawn::wire::WireServerDescriptor serverDesc = {};
    serverDesc.procs = &procs;
    serverDesc.serializer = &devNull;

    std::unique_ptr<dawn::wire::WireServer> wireServer(new dawn_wire::WireServer(serverDesc));
    wireServer->InjectInstance(sInstance->Get(), kInstanceObjectId, 0);

    static dawn::utils::TerribleCommandBuffer* mCommandBuffer =
        new dawn::utils::TerribleCommandBuffer();
    static dawn::wire::ChunkedCommandSerializer mSerializer =
        dawn::wire::ChunkedCommandSerializer(mCommandBuffer);
    mCommandBuffer->SetHandler(wireServer.get());

    dawn::wire::WireResult result = dawn::wire::SerializedData(program, mSerializer);

    mCommandBuffer->Flush();

    // Note: Deleting the server will release all created objects.
    // Deleted devices will wait for idle on destruction.
    wireServer = nullptr;
    return result == dawn::wire::WireResult::FatalError;
}

}  // namespace DawnLPMFuzzer
