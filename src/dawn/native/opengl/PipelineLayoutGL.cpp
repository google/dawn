// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/opengl/PipelineLayoutGL.h"

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/opengl/DeviceGL.h"

namespace dawn::native::opengl {

PipelineLayout::PipelineLayout(Device* device, const PipelineLayoutDescriptor* descriptor)
    : PipelineLayoutBase(device, descriptor) {
    GLuint uboIndex = 0;
    GLuint samplerIndex = 0;
    GLuint sampledTextureIndex = 0;
    GLuint ssboIndex = 0;
    GLuint storageTextureIndex = 0;

    for (BindGroupIndex group : IterateBitSet(GetBindGroupLayoutsMask())) {
        const BindGroupLayoutInternalBase* bgl = GetBindGroupLayout(group);
        mIndexInfo[group].resize(bgl->GetBindingCount());

        for (BindingIndex bindingIndex{0}; bindingIndex < bgl->GetBindingCount(); ++bindingIndex) {
            const BindingInfo& bindingInfo = bgl->GetBindingInfo(bindingIndex);
            switch (bindingInfo.bindingType) {
                case BindingInfoType::Buffer:
                    switch (bindingInfo.buffer.type) {
                        case wgpu::BufferBindingType::Uniform:
                            mIndexInfo[group][bindingIndex] = uboIndex;
                            uboIndex++;
                            break;
                        case wgpu::BufferBindingType::Storage:
                        case kInternalStorageBufferBinding:
                        case wgpu::BufferBindingType::ReadOnlyStorage:
                            mIndexInfo[group][bindingIndex] = ssboIndex;
                            ssboIndex++;
                            break;
                        case wgpu::BufferBindingType::Undefined:
                            DAWN_UNREACHABLE();
                    }
                    break;

                case BindingInfoType::Sampler:
                    mIndexInfo[group][bindingIndex] = samplerIndex;
                    samplerIndex++;
                    break;

                case BindingInfoType::Texture:
                case BindingInfoType::ExternalTexture:
                    mIndexInfo[group][bindingIndex] = sampledTextureIndex;
                    sampledTextureIndex++;
                    break;

                case BindingInfoType::StorageTexture:
                    mIndexInfo[group][bindingIndex] = storageTextureIndex;
                    storageTextureIndex++;
                    break;
            }
        }
    }

    mNumSamplers = samplerIndex;
    mNumSampledTextures = sampledTextureIndex;

    // Set internal uniform binding as the next unused uboIndex.
    mInternalUniformBinding = uboIndex;
}

const PipelineLayout::BindingIndexInfo& PipelineLayout::GetBindingIndexInfo() const {
    return mIndexInfo;
}

GLuint PipelineLayout::GetTextureUnitsUsed() const {
    return 0;
}

size_t PipelineLayout::GetNumSamplers() const {
    return mNumSamplers;
}

size_t PipelineLayout::GetNumSampledTextures() const {
    return mNumSampledTextures;
}

GLuint PipelineLayout::GetInternalUniformBinding() const {
    return mInternalUniformBinding;
}

}  // namespace dawn::native::opengl
