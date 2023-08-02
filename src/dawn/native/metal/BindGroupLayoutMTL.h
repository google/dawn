// Copyright 2020 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_METAL_BINDGROUPLAYOUTMTL_H_
#define SRC_DAWN_NATIVE_METAL_BINDGROUPLAYOUTMTL_H_

#include "dawn/common/SlabAllocator.h"
#include "dawn/native/BindGroupLayoutInternal.h"

namespace dawn::native::metal {

class BindGroup;
class Device;

class BindGroupLayout final : public BindGroupLayoutInternalBase {
  public:
    static Ref<BindGroupLayout> Create(DeviceBase* device,
                                       const BindGroupLayoutDescriptor* descriptor);

    Ref<BindGroup> AllocateBindGroup(Device* device, const BindGroupDescriptor* descriptor);
    void DeallocateBindGroup(BindGroup* bindGroup);

  private:
    BindGroupLayout(DeviceBase* device, const BindGroupLayoutDescriptor* descriptor);
    ~BindGroupLayout() override;

    SlabAllocator<BindGroup> mBindGroupAllocator;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_BINDGROUPLAYOUTMTL_H_
