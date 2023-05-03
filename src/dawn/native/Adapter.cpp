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

#include "dawn/native/Adapter.h"

#include <utility>

#include "dawn/native/PhysicalDevice.h"

namespace dawn::native {

AdapterBase::AdapterBase(const Ref<PhysicalDeviceBase>& physicalDevice)
    : mPhysicalDevice(std::move(physicalDevice)) {}

AdapterBase::~AdapterBase() = default;

PhysicalDeviceBase* AdapterBase::GetPhysicalDevice() {
    return mPhysicalDevice.Get();
}

InstanceBase* AdapterBase::APIGetInstance() const {
    return mPhysicalDevice->APIGetInstance();
}

bool AdapterBase::APIGetLimits(SupportedLimits* limits) const {
    return mPhysicalDevice->APIGetLimits(limits);
}

void AdapterBase::APIGetProperties(AdapterProperties* properties) const {
    mPhysicalDevice->APIGetProperties(properties);
}

bool AdapterBase::APIHasFeature(wgpu::FeatureName feature) const {
    return mPhysicalDevice->APIHasFeature(feature);
}

size_t AdapterBase::APIEnumerateFeatures(wgpu::FeatureName* features) const {
    return mPhysicalDevice->APIEnumerateFeatures(features);
}

DeviceBase* AdapterBase::APICreateDevice(const DeviceDescriptor* descriptor) {
    return mPhysicalDevice->CreateDevice(this, descriptor);
}

void AdapterBase::APIRequestDevice(const DeviceDescriptor* descriptor,
                                   WGPURequestDeviceCallback callback,
                                   void* userdata) {
    return mPhysicalDevice->RequestDevice(this, descriptor, callback, userdata);
}

const TogglesState& AdapterBase::GetTogglesState() const {
    return mPhysicalDevice->GetTogglesState();
}

}  // namespace dawn::native
