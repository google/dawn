// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/DynamicArrayState.h"

#include "dawn/common/Enumerator.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/Device.h"
#include "tint/api/common/resource_type.h"

namespace dawn::native {

namespace {

// Compute the tint::ResourceType that should be in the metadata buffer for the binding.
tint::ResourceType ComputeTypeId(const TextureViewBase* view) {
    if (view == nullptr) {
        return tint::ResourceType::kEmpty;
    }
    const TextureBase* texture = view->GetTexture();

    // TODO(https://crbug.com/435317394): In the future we should allow the same compatibility rules
    // that exist between TextureView and BGLEntry. This means that a depth texture can be either a
    // texture_depth_2d, or a texture_2d<f32> (unfilterable). We should also find a way to
    // differentiate unfilterable and filterable texture_2d<f32>.

    if (texture->IsMultisampledTexture()) {
        DAWN_ASSERT(view->GetDimension() == wgpu::TextureViewDimension::e2D);

        switch (view->GetAspects()) {
            case Aspect::Color:
                switch (view->GetFormat().GetAspectInfo(Aspect::Color).baseType) {
                    case TextureComponentType::Float:
                        return tint::ResourceType::kTextureMultisampled2d_f32;
                    case TextureComponentType::Uint:
                        return tint::ResourceType::kTextureMultisampled2d_u32;
                    case TextureComponentType::Sint:
                        return tint::ResourceType::kTextureMultisampled2d_i32;
                    default:
                        DAWN_UNREACHABLE();
                }

            case Aspect::Depth:
                return tint::ResourceType::kTextureDepthMultisampled2d;
            default:
                DAWN_UNREACHABLE();
        }
    }

    if (view->GetAspects() == Aspect::Depth) {
        DAWN_ASSERT(!texture->IsMultisampledTexture());

        switch (view->GetDimension()) {
            case wgpu::TextureViewDimension::e2D:
                return tint::ResourceType::kTextureDepth2d;
            case wgpu::TextureViewDimension::e2DArray:
                return tint::ResourceType::kTextureDepth2dArray;
            case wgpu::TextureViewDimension::Cube:
                return tint::ResourceType::kTextureDepthCube;
            case wgpu::TextureViewDimension::CubeArray:
                return tint::ResourceType::kTextureDepthCubeArray;
            default:
                DAWN_UNREACHABLE();
        }
    }

    switch (view->GetFormat().GetAspectInfo(view->GetAspects()).baseType) {
        case TextureComponentType::Float:
            switch (view->GetDimension()) {
                case wgpu::TextureViewDimension::e1D:
                    return tint::ResourceType::kTexture1d_f32;
                case wgpu::TextureViewDimension::e2D:
                    return tint::ResourceType::kTexture2d_f32;
                case wgpu::TextureViewDimension::e2DArray:
                    return tint::ResourceType::kTexture2dArray_f32;
                case wgpu::TextureViewDimension::Cube:
                    return tint::ResourceType::kTextureCube_f32;
                case wgpu::TextureViewDimension::CubeArray:
                    return tint::ResourceType::kTextureCubeArray_f32;
                case wgpu::TextureViewDimension::e3D:
                    return tint::ResourceType::kTexture3d_f32;
                default:
                    DAWN_UNREACHABLE();
            }
        case TextureComponentType::Uint:
            switch (view->GetDimension()) {
                case wgpu::TextureViewDimension::e1D:
                    return tint::ResourceType::kTexture1d_u32;
                case wgpu::TextureViewDimension::e2D:
                    return tint::ResourceType::kTexture2d_u32;
                case wgpu::TextureViewDimension::e2DArray:
                    return tint::ResourceType::kTexture2dArray_u32;
                case wgpu::TextureViewDimension::Cube:
                    return tint::ResourceType::kTextureCube_u32;
                case wgpu::TextureViewDimension::CubeArray:
                    return tint::ResourceType::kTextureCubeArray_u32;
                case wgpu::TextureViewDimension::e3D:
                    return tint::ResourceType::kTexture3d_u32;
                default:
                    DAWN_UNREACHABLE();
            }
        case TextureComponentType::Sint:
            switch (view->GetDimension()) {
                case wgpu::TextureViewDimension::e1D:
                    return tint::ResourceType::kTexture1d_i32;
                case wgpu::TextureViewDimension::e2D:
                    return tint::ResourceType::kTexture2d_i32;
                case wgpu::TextureViewDimension::e2DArray:
                    return tint::ResourceType::kTexture2dArray_i32;
                case wgpu::TextureViewDimension::Cube:
                    return tint::ResourceType::kTextureCube_i32;
                case wgpu::TextureViewDimension::CubeArray:
                    return tint::ResourceType::kTextureCubeArray_i32;
                case wgpu::TextureViewDimension::e3D:
                    return tint::ResourceType::kTexture3d_i32;
                default:
                    DAWN_UNREACHABLE();
            }
        default:
            DAWN_UNREACHABLE();
    }
}

}  // anonymous namespace

DynamicArrayState::DynamicArrayState(BindingIndex size) {
    mBindings.resize(size);

    DAWN_ASSERT(ComputeTypeId(nullptr) == BindingState{}.typeId);
    mBindingState.resize(size);
}

MaybeError DynamicArrayState::Initialize(DeviceBase* device) {
    // TODO(https://crbug.com/435317394): Default bindings will be included in mBindings in the
    // future such that we should use the dynamicArraySize passed in the BindGroup creation instead
    // of the size of mBindings.
    uint32_t metadataArrayLength = uint32_t(mBindings.size());

    // Create a storage buffer that will hold the shader-visible metadata for the dynamic array.
    BufferDescriptor metadataDesc{
        .label = "binding array metadata",
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst,
        .size = sizeof(uint32_t) * (metadataArrayLength + 1),
        .mappedAtCreation = true,
    };
    DAWN_TRY_ASSIGN(mMetadataBuffer, device->CreateBuffer(&metadataDesc));

    // Initialize the metadata buffer with the arrayLength and a bunch of zeroes that correspond to
    // empty entries.
    DAWN_ASSERT(uint32_t(tint::ResourceType::kEmpty) == 0);
    // TODO(https://crbug.com/435317394): We could rely on zero initialization if it is enabled, and
    // also apply the initial dirty bindings in this mapping instead of one the first use of the
    // dynamic binding array.
    uint32_t* data = static_cast<uint32_t*>(mMetadataBuffer->GetMappedRange(0, metadataDesc.size));
    *data = metadataArrayLength;
    memset(data + 1, 0, metadataDesc.size - sizeof(uint32_t));
    DAWN_TRY(mMetadataBuffer->Unmap());

    return {};
}

void DynamicArrayState::Destroy() {
    DAWN_ASSERT(!mDestroyed);

    for (auto [i, view] : Enumerate(mBindings)) {
        if (view != nullptr) {
            view->GetTexture()->RemoveDynamicArraySlot(this, i);
        }
    }

    mBindings.clear();
    mBindingState.clear();
    mDirtyBindings.clear();
    mMetadataBuffer->Destroy();
    mMetadataBuffer = nullptr;

    mDestroyed = true;
}

BindingIndex DynamicArrayState::GetSize() const {
    DAWN_ASSERT(!mDestroyed);
    return mBindings.size();
}

ityp::span<BindingIndex, const Ref<TextureViewBase>> DynamicArrayState::GetBindings() const {
    DAWN_ASSERT(!mDestroyed);
    return {mBindings.data(), mBindings.size()};
}

BufferBase* DynamicArrayState::GetMetadataBuffer() const {
    DAWN_ASSERT(!mDestroyed);
    return mMetadataBuffer.Get();
}

bool DynamicArrayState::IsDestroyed() const {
    return mDestroyed;
}

void DynamicArrayState::Update(BindingIndex i, TextureViewBase* view) {
    DAWN_ASSERT(!mDestroyed);
    if (mBindings[i] == view) {
        return;
    }

    // Update the mBindings slot but also the mapping to the slot that are stored in the textures.
    if (mBindings[i] != nullptr) {
        mBindings[i]->GetTexture()->RemoveDynamicArraySlot(this, i);
    }
    if (view != nullptr) {
        view->GetTexture()->AddDynamicArraySlot(this, i);
    }
    mBindings[i] = view;

    // Update the mBindingState with information for the updated binding.
    tint::ResourceType typeId = ComputeTypeId(view);
    bool pinned = view != nullptr && view->GetTexture()->HasPinnedUsage();

    BindingState& state = mBindingState[i];
    if (state.typeId != typeId || state.pinned != pinned) {
        state.typeId = typeId;
        state.pinned = pinned;
        MarkStateDirty(i);
    }
}

void DynamicArrayState::OnPinned(BindingIndex i, TextureBase* texture) {
    DAWN_ASSERT(!mDestroyed);
    DAWN_ASSERT(mBindings[i] != nullptr);
    DAWN_ASSERT(mBindings[i]->GetTexture() == texture);
    DAWN_ASSERT(!mBindingState[i].pinned);
    mBindingState[i].pinned = true;
    MarkStateDirty(i);
}

void DynamicArrayState::OnUnpinned(BindingIndex i, TextureBase* texture) {
    DAWN_ASSERT(!mDestroyed);
    DAWN_ASSERT(mBindings[i] != nullptr);
    DAWN_ASSERT(mBindings[i]->GetTexture() == texture);
    DAWN_ASSERT(mBindingState[i].pinned);
    mBindingState[i].pinned = false;
    MarkStateDirty(i);
}

bool DynamicArrayState::HasDirtyBindings() const {
    return !mDirtyBindings.empty();
}

std::vector<DynamicArrayState::BindingStateUpdate> DynamicArrayState::AcquireDirtyBindingUpdates() {
    DAWN_ASSERT(!mDestroyed);

    std::vector<BindingStateUpdate> updates;
    for (BindingIndex dirtyIndex : mDirtyBindings) {
        DAWN_ASSERT(mBindingState[dirtyIndex].dirty);
        mBindingState[dirtyIndex].dirty = false;

        tint::ResourceType effectiveType = mBindingState[dirtyIndex].pinned
                                               ? mBindingState[dirtyIndex].typeId
                                               : tint::ResourceType::kEmpty;

        size_t offset = sizeof(uint32_t) * (uint32_t(dirtyIndex) + 1);
        updates.push_back({
            .offset = uint32_t(offset),
            .data = uint32_t(effectiveType),
        });
    }
    mDirtyBindings.clear();

    return updates;
}

void DynamicArrayState::MarkStateDirty(BindingIndex i) {
    if (!mBindingState[i].dirty) {
        mDirtyBindings.push_back(i);
        mBindingState[i].dirty = true;
    }
}

}  // namespace dawn::native
