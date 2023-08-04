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

#include "dawn/native/SharedTextureMemory.h"

#include "dawn/native/Device.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

// static
SharedTextureMemoryBase* SharedTextureMemoryBase::MakeError(
    DeviceBase* device,
    const SharedTextureMemoryDescriptor* descriptor) {
    return new SharedTextureMemoryBase(device, descriptor, ObjectBase::kError);
}

SharedTextureMemoryBase::SharedTextureMemoryBase(DeviceBase* device,
                                                 const SharedTextureMemoryDescriptor* descriptor,
                                                 ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag, descriptor->label) {}

ObjectType SharedTextureMemoryBase::GetType() const {
    return ObjectType::SharedTextureMemory;
}

void SharedTextureMemoryBase::DestroyImpl() {}

void SharedTextureMemoryBase::APIGetProperties(SharedTextureMemoryProperties* properties) const {
    DAWN_UNUSED(GetDevice()->ConsumedError(DAWN_UNIMPLEMENTED_ERROR("Not implemented")));
}

TextureBase* SharedTextureMemoryBase::APICreateTexture(const TextureDescriptor* descriptor) {
    DAWN_UNUSED(GetDevice()->ConsumedError(DAWN_UNIMPLEMENTED_ERROR("Not implemented")));
    return TextureBase::MakeError(GetDevice(), descriptor);
}

void SharedTextureMemoryBase::APIBeginAccess(TextureBase* texture,
                                             const BeginAccessDescriptor* descriptor) {
    DAWN_UNUSED(GetDevice()->ConsumedError(DAWN_UNIMPLEMENTED_ERROR("Not implemented")));
}

void SharedTextureMemoryBase::APIEndAccess(TextureBase* texture, EndAccessState* state) {
    DAWN_UNUSED(GetDevice()->ConsumedError(DAWN_UNIMPLEMENTED_ERROR("Not implemented")));
}

}  // namespace dawn::native
