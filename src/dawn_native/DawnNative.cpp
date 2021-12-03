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

#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/Device.h"
#include "dawn_native/Instance.h"
#include "dawn_native/Texture.h"
#include "dawn_platform/DawnPlatform.h"

// Contains the entry-points into dawn_native

namespace dawn_native {

    const DawnProcTable& GetProcsAutogen();

    const DawnProcTable& GetProcs() {
        return GetProcsAutogen();
    }

    std::vector<const char*> GetTogglesUsed(WGPUDevice device) {
        return FromAPI(device)->GetTogglesUsed();
    }

    // Adapter

    Adapter::Adapter() = default;

    Adapter::Adapter(AdapterBase* impl) : mImpl(impl) {
    }

    Adapter::~Adapter() {
        mImpl = nullptr;
    }

    Adapter::Adapter(const Adapter& other) = default;
    Adapter& Adapter::operator=(const Adapter& other) = default;

    void Adapter::GetProperties(wgpu::AdapterProperties* properties) const {
        properties->backendType = mImpl->GetBackendType();
        properties->adapterType = mImpl->GetAdapterType();
        properties->driverDescription = mImpl->GetDriverDescription().c_str();
        properties->deviceID = mImpl->GetPCIInfo().deviceId;
        properties->vendorID = mImpl->GetPCIInfo().vendorId;
        properties->name = mImpl->GetPCIInfo().name.c_str();
    }

    BackendType Adapter::GetBackendType() const {
        switch (mImpl->GetBackendType()) {
            case wgpu::BackendType::D3D12:
                return BackendType::D3D12;
            case wgpu::BackendType::Metal:
                return BackendType::Metal;
            case wgpu::BackendType::Null:
                return BackendType::Null;
            case wgpu::BackendType::OpenGL:
                return BackendType::OpenGL;
            case wgpu::BackendType::Vulkan:
                return BackendType::Vulkan;
            case wgpu::BackendType::OpenGLES:
                return BackendType::OpenGLES;

            case wgpu::BackendType::D3D11:
            case wgpu::BackendType::WebGPU:
                break;
        }
        UNREACHABLE();
    }

    DeviceType Adapter::GetDeviceType() const {
        switch (mImpl->GetAdapterType()) {
            case wgpu::AdapterType::DiscreteGPU:
                return DeviceType::DiscreteGPU;
            case wgpu::AdapterType::IntegratedGPU:
                return DeviceType::IntegratedGPU;
            case wgpu::AdapterType::CPU:
                return DeviceType::CPU;
            case wgpu::AdapterType::Unknown:
                return DeviceType::Unknown;
        }
        UNREACHABLE();
    }

    const PCIInfo& Adapter::GetPCIInfo() const {
        return mImpl->GetPCIInfo();
    }

    std::vector<const char*> Adapter::GetSupportedFeatures() const {
        FeaturesSet supportedFeaturesSet = mImpl->GetSupportedFeatures();
        return supportedFeaturesSet.GetEnabledFeatureNames();
    }

    WGPUDeviceProperties Adapter::GetAdapterProperties() const {
        return mImpl->GetAdapterProperties();
    }

    bool Adapter::GetLimits(WGPUSupportedLimits* limits) const {
        return mImpl->GetLimits(FromAPI(limits));
    }

    void Adapter::SetUseTieredLimits(bool useTieredLimits) {
        mImpl->SetUseTieredLimits(useTieredLimits);
    }

    bool Adapter::SupportsExternalImages() const {
        return mImpl->SupportsExternalImages();
    }

    Adapter::operator bool() const {
        return mImpl != nullptr;
    }

    WGPUDevice Adapter::CreateDevice(const DawnDeviceDescriptor* deviceDescriptor) {
        return ToAPI(mImpl->CreateDevice(deviceDescriptor));
    }

    void Adapter::RequestDevice(const DawnDeviceDescriptor* descriptor,
                                WGPURequestDeviceCallback callback,
                                void* userdata) {
        mImpl->RequestDevice(descriptor, callback, userdata);
    }

    void Adapter::ResetInternalDeviceForTesting() {
        mImpl->ResetInternalDeviceForTesting();
    }

    // AdapterDiscoverOptionsBase

    AdapterDiscoveryOptionsBase::AdapterDiscoveryOptionsBase(WGPUBackendType type)
        : backendType(type) {
    }

    // Instance

    Instance::Instance() : mImpl(InstanceBase::Create()) {
    }

    Instance::~Instance() {
        if (mImpl != nullptr) {
            mImpl->Release();
            mImpl = nullptr;
        }
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
        if (enableBackendValidation) {
            mImpl->SetBackendValidationLevel(BackendValidationLevel::Full);
        }
    }

    void Instance::SetBackendValidationLevel(BackendValidationLevel level) {
        mImpl->SetBackendValidationLevel(level);
    }

    void Instance::EnableBeginCaptureOnStartup(bool beginCaptureOnStartup) {
        mImpl->EnableBeginCaptureOnStartup(beginCaptureOnStartup);
    }

    void Instance::SetPlatform(dawn_platform::Platform* platform) {
        mImpl->SetPlatform(platform);
    }

    WGPUInstance Instance::Get() const {
        return ToAPI(mImpl);
    }

    size_t GetLazyClearCountForTesting(WGPUDevice device) {
        return FromAPI(device)->GetLazyClearCountForTesting();
    }

    size_t GetDeprecationWarningCountForTesting(WGPUDevice device) {
        return FromAPI(device)->GetDeprecationWarningCountForTesting();
    }

    bool IsTextureSubresourceInitialized(WGPUTexture texture,
                                         uint32_t baseMipLevel,
                                         uint32_t levelCount,
                                         uint32_t baseArrayLayer,
                                         uint32_t layerCount,
                                         WGPUTextureAspect cAspect) {
        TextureBase* textureBase = FromAPI(texture);

        Aspect aspect =
            ConvertAspect(textureBase->GetFormat(), static_cast<wgpu::TextureAspect>(cAspect));
        SubresourceRange range(aspect, {baseArrayLayer, layerCount}, {baseMipLevel, levelCount});
        return textureBase->IsSubresourceContentInitialized(range);
    }

    std::vector<const char*> GetProcMapNamesForTestingInternal();

    std::vector<const char*> GetProcMapNamesForTesting() {
        return GetProcMapNamesForTestingInternal();
    }

    DAWN_NATIVE_EXPORT bool DeviceTick(WGPUDevice device) {
        return FromAPI(device)->APITick();
    }

    // ExternalImageDescriptor

    ExternalImageDescriptor::ExternalImageDescriptor(ExternalImageType type) : type(type) {
    }

    // ExternalImageExportInfo

    ExternalImageExportInfo::ExternalImageExportInfo(ExternalImageType type) : type(type) {
    }

    const char* GetObjectLabelForTesting(void* objectHandle) {
        ApiObjectBase* object = reinterpret_cast<ApiObjectBase*>(objectHandle);
        return object->GetLabel().c_str();
    }

    uint64_t GetAllocatedSizeForTesting(WGPUBuffer buffer) {
        return FromAPI(buffer)->GetAllocatedSize();
    }

    bool BindGroupLayoutBindingsEqualForTesting(WGPUBindGroupLayout a, WGPUBindGroupLayout b) {
        bool excludePipelineCompatibiltyToken = true;
        return FromAPI(a)->IsLayoutEqual(FromAPI(b), excludePipelineCompatibiltyToken);
    }

}  // namespace dawn_native
