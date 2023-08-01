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

#include "dawn/native/d3d11/BindGroupD3D11.h"

#include <memory>

#include "dawn/native/Texture.h"
#include "dawn/native/d3d11/BindGroupLayoutD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"

namespace dawn::native::d3d11 {

// static
Ref<BindGroup> BindGroup::Create(Device* device, const BindGroupDescriptor* descriptor) {
    return ToBackend(descriptor->layout->GetInternalBindGroupLayout())
        ->AllocateBindGroup(device, descriptor);
}

BindGroup::BindGroup(Device* device, const BindGroupDescriptor* descriptor)
    : BindGroupBase(this, device, descriptor) {}

BindGroup::~BindGroup() = default;

void BindGroup::DestroyImpl() {
    BindGroupBase::DestroyImpl();
    ToBackend(GetLayout()->GetInternalBindGroupLayout())->DeallocateBindGroup(this);
}

}  // namespace dawn::native::d3d11
