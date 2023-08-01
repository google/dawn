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

#include "dawn/native/d3d11/BindGroupLayoutD3D11.h"

#include "dawn/native/d3d11/DeviceD3D11.h"

namespace dawn::native::d3d11 {

ResultOrError<Ref<BindGroupLayout>> BindGroupLayout::Create(
    Device* device,
    const BindGroupLayoutDescriptor* descriptor) {
    return AcquireRef(new BindGroupLayout(device, descriptor));
}

BindGroupLayout::BindGroupLayout(Device* device, const BindGroupLayoutDescriptor* descriptor)
    : BindGroupLayoutInternalBase(device, descriptor),
      mBindGroupAllocator(MakeFrontendBindGroupAllocator<BindGroup>(4096)) {}

Ref<BindGroup> BindGroupLayout::AllocateBindGroup(Device* device,
                                                  const BindGroupDescriptor* descriptor) {
    return AcquireRef(mBindGroupAllocator.Allocate(device, descriptor));
}

void BindGroupLayout::DeallocateBindGroup(BindGroup* bindGroup) {
    mBindGroupAllocator.Deallocate(bindGroup);
}

}  // namespace dawn::native::d3d11
