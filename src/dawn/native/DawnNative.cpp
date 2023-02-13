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

#include "dawn/native/DawnNative.h"

#include <vector>

#include "dawn/common/Log.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/Device.h"
#include "dawn/native/Instance.h"
#include "dawn/native/Texture.h"
#include "dawn/platform/DawnPlatform.h"

// Contains the entry-points into dawn_native

namespace dawn::native {

namespace {
struct ComboDeprecatedDawnDeviceDescriptor : DeviceDescriptor {
    explicit ComboDeprecatedDawnDeviceDescriptor(const DawnDeviceDescriptor* deviceDescriptor) {
        dawn::WarningLog() << "DawnDeviceDescriptor is deprecated. Please use "
                              "WGPUDeviceDescriptor instead.";

        DeviceDescriptor* desc = this;

        if (deviceDescriptor != nullptr) {
            desc->nextInChain = &mTogglesDesc;
            mTogglesDesc.enabledToggles = deviceDescriptor->forceEnabledToggles.data();
            mTogglesDesc.enabledTogglesCount = deviceDescriptor->forceEnabledToggles.size();
            mTogglesDesc.disabledToggles = deviceDescriptor->forceDisabledToggles.data();
            mTogglesDesc.disabledTogglesCount = deviceDescriptor->forceDisabledToggles.size();

            desc->requiredLimits =
                reinterpret_cast<const RequiredLimits*>(deviceDescriptor->requiredLimits);

            FeaturesInfo featuresInfo;
            for (const char* featureStr : deviceDescriptor->requiredFeatures) {
                mRequiredFeatures.push_back(featuresInfo.FeatureNameToAPIEnum(featureStr));
            }
            desc->requiredFeatures = mRequiredFeatures.data();
            desc->requiredFeaturesCount = mRequiredFeatures.size();
        }
    }

    DawnTogglesDescriptor mTogglesDesc = {};
    std::vector<wgpu::FeatureName> mRequiredFeatures = {};
};
}  // namespace

const DawnProcTable& GetProcsAutogen();

const DawnProcTable& GetProcs() {
    return GetProcsAutogen();
}

std::vector<const char*> GetTogglesUsed(WGPUDevice device) {
    return FromAPI(device)->GetTogglesUsed();
}

// DawnDeviceDescriptor

DawnDeviceDescriptor::DawnDeviceDescriptor() = default;

DawnDeviceDescriptor::~DawnDeviceDescriptor() = default;

// Adapter

Adapter::Adapter() = default;

Adapter::Adapter(AdapterBase* impl) : mImpl(impl) {
    if (mImpl != nullptr) {
        mImpl->Reference();
    }
}

Adapter::~Adapter() {
    if (mImpl != nullptr) {
        mImpl->Release();
    }
    mImpl = nullptr;
}

Adapter::Adapter(const Adapter& other) : Adapter(other.mImpl) {}

Adapter& Adapter::operator=(const Adapter& other) {
    if (this != &other) {
        if (mImpl) {
            mImpl->Release();
        }
        mImpl = other.mImpl;
        if (mImpl) {
            mImpl->Reference();
        }
    }
    return *this;
}

void Adapter::GetProperties(wgpu::AdapterProperties* properties) const {
    GetProperties(reinterpret_cast<WGPUAdapterProperties*>(properties));
}

void Adapter::GetProperties(WGPUAdapterProperties* properties) const {
    mImpl->APIGetProperties(FromAPI(properties));
}

WGPUAdapter Adapter::Get() const {
    return ToAPI(mImpl);
}

std::vector<const char*> Adapter::GetSupportedFeatures() const {
    FeaturesSet supportedFeaturesSet = mImpl->GetSupportedFeatures();
    return supportedFeaturesSet.GetEnabledFeatureNames();
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
    ComboDeprecatedDawnDeviceDescriptor desc(deviceDescriptor);
    return ToAPI(mImpl->APICreateDevice(&desc));
}

WGPUDevice Adapter::CreateDevice(const wgpu::DeviceDescriptor* deviceDescriptor) {
    return CreateDevice(reinterpret_cast<const WGPUDeviceDescriptor*>(deviceDescriptor));
}

WGPUDevice Adapter::CreateDevice(const WGPUDeviceDescriptor* deviceDescriptor) {
    return ToAPI(mImpl->APICreateDevice(FromAPI(deviceDescriptor)));
}

void Adapter::RequestDevice(const DawnDeviceDescriptor* descriptor,
                            WGPURequestDeviceCallback callback,
                            void* userdata) {
    ComboDeprecatedDawnDeviceDescriptor desc(descriptor);
    mImpl->APIRequestDevice(&desc, callback, userdata);
}

void Adapter::RequestDevice(const wgpu::DeviceDescriptor* descriptor,
                            WGPURequestDeviceCallback callback,
                            void* userdata) {
    mImpl->APIRequestDevice(reinterpret_cast<const DeviceDescriptor*>(descriptor), callback,
                            userdata);
}

void Adapter::RequestDevice(const WGPUDeviceDescriptor* descriptor,
                            WGPURequestDeviceCallback callback,
                            void* userdata) {
    mImpl->APIRequestDevice(reinterpret_cast<const DeviceDescriptor*>(descriptor), callback,
                            userdata);
}

void Adapter::ResetInternalDeviceForTesting() {
    mImpl->ResetInternalDeviceForTesting();
}

// AdapterDiscoverOptionsBase

AdapterDiscoveryOptionsBase::AdapterDiscoveryOptionsBase(WGPUBackendType type)
    : backendType(type) {}

// Instance

Instance::Instance(const WGPUInstanceDescriptor* desc)
    : mImpl(APICreateInstance(reinterpret_cast<const InstanceDescriptor*>(desc))) {}

Instance::~Instance() {
    if (mImpl != nullptr) {
        mImpl->APIRelease();
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
    for (const Ref<AdapterBase>& adapter : mImpl->GetAdapters()) {
        adapters.push_back(Adapter(adapter.Get()));
    }
    return adapters;
}

const ToggleInfo* Instance::GetToggleInfo(const char* toggleName) {
    return mImpl->GetToggleInfo(toggleName);
}

const FeatureInfo* Instance::GetFeatureInfo(WGPUFeatureName feature) {
    return mImpl->GetFeatureInfo(static_cast<wgpu::FeatureName>(feature));
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

void Instance::EnableAdapterBlocklist(bool enable) {
    mImpl->EnableAdapterBlocklist(enable);
}

// TODO(dawn:1374) Deprecate this once it is passed via the descriptor.
void Instance::SetPlatform(dawn::platform::Platform* platform) {
    mImpl->SetPlatform(platform);
}

uint64_t Instance::GetDeviceCountForTesting() const {
    return mImpl->GetDeviceCountForTesting();
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

size_t GetAdapterCountForTesting(WGPUInstance instance) {
    return FromAPI(instance)->GetAdapters().size();
}

bool IsTextureSubresourceInitialized(WGPUTexture texture,
                                     uint32_t baseMipLevel,
                                     uint32_t levelCount,
                                     uint32_t baseArrayLayer,
                                     uint32_t layerCount,
                                     WGPUTextureAspect cAspect) {
    TextureBase* textureBase = FromAPI(texture);
    if (textureBase->IsError()) {
        return false;
    }

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

ExternalImageDescriptor::ExternalImageDescriptor(ExternalImageType type) : mType(type) {}

ExternalImageType ExternalImageDescriptor::GetType() const {
    return mType;
}

// ExternalImageExportInfo

ExternalImageExportInfo::ExternalImageExportInfo(ExternalImageType type) : mType(type) {}

ExternalImageType ExternalImageExportInfo::GetType() const {
    return mType;
}

bool CheckIsErrorForTesting(void* objectHandle) {
    return reinterpret_cast<ErrorMonad*>(objectHandle)->IsError();
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

}  // namespace dawn::native
