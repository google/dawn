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
#include "dawn_platform/DawnPlatform.h"

// Contains the entry-points into dawn_native

namespace dawn_native {

    DawnProcTable GetProcsAutogen();

    DawnProcTable GetProcs() {
        return GetProcsAutogen();
    }

    std::vector<const char*> GetTogglesUsed(WGPUDevice device) {
        const dawn_native::DeviceBase* deviceBase =
            reinterpret_cast<const dawn_native::DeviceBase*>(device);
        return deviceBase->GetTogglesUsed();
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

    DeviceType Adapter::GetDeviceType() const {
        return mImpl->GetDeviceType();
    }

    const PCIInfo& Adapter::GetPCIInfo() const {
        return mImpl->GetPCIInfo();
    }

    std::vector<const char*> Adapter::GetSupportedExtensions() const {
        ExtensionsSet supportedExtensionsSet = mImpl->GetSupportedExtensions();
        return supportedExtensionsSet.GetEnabledExtensionNames();
    }

    Adapter::operator bool() const {
        return mImpl != nullptr;
    }

    WGPUDevice Adapter::CreateDevice(const DeviceDescriptor* deviceDescriptor) {
        return reinterpret_cast<WGPUDevice>(mImpl->CreateDevice(deviceDescriptor));
    }

    // AdapterDiscoverOptionsBase

    AdapterDiscoveryOptionsBase::AdapterDiscoveryOptionsBase(BackendType type) : backendType(type) {
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

    bool Instance::DiscoverAdapters(const AdapterDiscoveryOptionsBase* options) {
        return mImpl->DiscoverAdapters(options);
    }

    std::vector<Adapter> Instance::GetAdapters() const {
        // Adapters are owned by mImpl so it is safe to return non RAII pointers to them
        std::vector<Adapter> adapters;
        for (const std::unique_ptr<AdapterBase>& adapter : mImpl->GetAdapters()) {
            adapters.push_back({adapter.get()});
        }
        return adapters;
    }

    const ToggleInfo* Instance::GetToggleInfo(const char* toggleName) {
        return mImpl->GetToggleInfo(toggleName);
    }

    void Instance::EnableBackendValidation(bool enableBackendValidation) {
        mImpl->EnableBackendValidation(enableBackendValidation);
    }

    bool Instance::IsBackendValidationEnabled() const {
        return mImpl->IsBackendValidationEnabled();
    }

    void Instance::EnableBeginCaptureOnStartup(bool beginCaptureOnStartup) {
        mImpl->EnableBeginCaptureOnStartup(beginCaptureOnStartup);
    }

    bool Instance::IsBeginCaptureOnStartupEnabled() const {
        return mImpl->IsBeginCaptureOnStartupEnabled();
    }

    void Instance::SetPlatform(dawn_platform::Platform* platform) {
        mImpl->SetPlatform(platform);
    }

    dawn_platform::Platform* Instance::GetPlatform() const {
        return mImpl->GetPlatform();
    }

    size_t GetLazyClearCountForTesting(WGPUDevice device) {
        dawn_native::DeviceBase* deviceBase = reinterpret_cast<dawn_native::DeviceBase*>(device);
        return deviceBase->GetLazyClearCountForTesting();
    }

    std::vector<const char*> GetProcMapNamesForTestingInternal();

    std::vector<const char*> GetProcMapNamesForTesting() {
        return GetProcMapNamesForTestingInternal();
    }

}  // namespace dawn_native
