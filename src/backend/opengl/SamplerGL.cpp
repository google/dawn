// Copyright 2017 The NXT Authors
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

#include "backend/opengl/SamplerGL.h"

#include "common/Assert.h"

namespace backend { namespace opengl {

    namespace {
        GLenum MagFilterMode(nxt::FilterMode filter) {
            switch (filter) {
                case nxt::FilterMode::Nearest:
                    return GL_NEAREST;
                case nxt::FilterMode::Linear:
                    return GL_LINEAR;
                default:
                    UNREACHABLE();
            }
        }

        GLenum MinFilterMode(nxt::FilterMode minFilter, nxt::FilterMode mipMapFilter) {
            switch (minFilter) {
                case nxt::FilterMode::Nearest:
                    switch (mipMapFilter) {
                        case nxt::FilterMode::Nearest:
                            return GL_NEAREST_MIPMAP_NEAREST;
                        case nxt::FilterMode::Linear:
                            return GL_NEAREST_MIPMAP_LINEAR;
                        default:
                            UNREACHABLE();
                    }
                case nxt::FilterMode::Linear:
                    switch (mipMapFilter) {
                        case nxt::FilterMode::Nearest:
                            return GL_LINEAR_MIPMAP_NEAREST;
                        case nxt::FilterMode::Linear:
                            return GL_LINEAR_MIPMAP_LINEAR;
                        default:
                            UNREACHABLE();
                    }
                default:
                    UNREACHABLE();
            }
        }

        GLenum WrapMode(nxt::AddressMode mode) {
            switch (mode) {
                case nxt::AddressMode::Repeat:
                    return GL_REPEAT;
                case nxt::AddressMode::MirroredRepeat:
                    return GL_MIRRORED_REPEAT;
                case nxt::AddressMode::ClampToEdge:
                    return GL_CLAMP_TO_EDGE;
                default:
                    UNREACHABLE();
            }
        }

    }  // namespace

    Sampler::Sampler(SamplerBuilder* builder) : SamplerBase(builder) {
        glGenSamplers(1, &mHandle);
        glSamplerParameteri(mHandle, GL_TEXTURE_MAG_FILTER, MagFilterMode(builder->GetMagFilter()));
        glSamplerParameteri(mHandle, GL_TEXTURE_MIN_FILTER,
                            MinFilterMode(builder->GetMinFilter(), builder->GetMipMapFilter()));
        glSamplerParameteri(mHandle, GL_TEXTURE_WRAP_R, WrapMode(builder->GetAddressModeW()));
        glSamplerParameteri(mHandle, GL_TEXTURE_WRAP_S, WrapMode(builder->GetAddressModeU()));
        glSamplerParameteri(mHandle, GL_TEXTURE_WRAP_T, WrapMode(builder->GetAddressModeV()));
    }

    GLuint Sampler::GetHandle() const {
        return mHandle;
    }

}}  // namespace backend::opengl
