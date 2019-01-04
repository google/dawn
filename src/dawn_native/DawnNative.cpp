// Copyright 2018 The Dawn Authors
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

#include "dawn_native/DawnNative.h"
#include "dawn_native/Device.h"
#include "dawn_native/Instance.h"

// Contains the entry-points into dawn_native

namespace dawn_native {

    dawnProcTable GetProcsAutogen();

    dawnProcTable GetProcs() {
        return GetProcsAutogen();
    }

    const PCIInfo& GetPCIInfo(dawnDevice device) {
        DeviceBase* deviceBase = reinterpret_cast<DeviceBase*>(device);
        return deviceBase->GetPCIInfo();
    }

    // Adapter

    Adapter::Adapter() = default;

    Adapter::Adapter(AdapterBase* impl) : mImpl(impl) {
    }

    Adapter::~Adapter() {
        mImpl = nullptr;
    }

    BackendType Adapter::GetBackendType() const {
        return mImpl->GetBackendType();
    }

    const PCIInfo& Adapter::GetPCIInfo() const {
        return mImpl->GetPCIInfo();
    }

    dawnDevice Adapter::CreateDevice() {
        return reinterpret_cast<dawnDevice>(mImpl->CreateDevice());
    }

    // Instance

    Instance::Instance() : mImpl(new InstanceBase()) {
    }

    Instance::~Instance() {
        delete mImpl;
        mImpl = nullptr;
    }

    void Instance::DiscoverDefaultAdapters() {
        mImpl->DiscoverDefaultAdapters();
    }

    std::vector<Adapter> Instance::GetAdapters() const {
        // Adapters are owned by mImpl so it is safe to return non RAII pointers to them
        std::vector<Adapter> adapters;
        for (const std::unique_ptr<AdapterBase>& adapter : mImpl->GetAdapters()) {
            adapters.push_back({adapter.get()});
        }
        return adapters;
    }

}  // namespace dawn_native
