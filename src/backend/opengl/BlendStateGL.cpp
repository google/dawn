// Copyright 2017 The Dawn Authors
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

#include "backend/opengl/BlendStateGL.h"

#include "common/Assert.h"

namespace backend { namespace opengl {

    namespace {
        GLenum GLBlendFactor(nxt::BlendFactor factor, bool alpha) {
            switch (factor) {
                case nxt::BlendFactor::Zero:
                    return GL_ZERO;
                case nxt::BlendFactor::One:
                    return GL_ONE;
                case nxt::BlendFactor::SrcColor:
                    return GL_SRC_COLOR;
                case nxt::BlendFactor::OneMinusSrcColor:
                    return GL_ONE_MINUS_SRC_COLOR;
                case nxt::BlendFactor::SrcAlpha:
                    return GL_SRC_ALPHA;
                case nxt::BlendFactor::OneMinusSrcAlpha:
                    return GL_ONE_MINUS_SRC_ALPHA;
                case nxt::BlendFactor::DstColor:
                    return GL_DST_COLOR;
                case nxt::BlendFactor::OneMinusDstColor:
                    return GL_ONE_MINUS_DST_COLOR;
                case nxt::BlendFactor::DstAlpha:
                    return GL_DST_ALPHA;
                case nxt::BlendFactor::OneMinusDstAlpha:
                    return GL_ONE_MINUS_DST_ALPHA;
                case nxt::BlendFactor::SrcAlphaSaturated:
                    return GL_SRC_ALPHA_SATURATE;
                case nxt::BlendFactor::BlendColor:
                    return alpha ? GL_CONSTANT_ALPHA : GL_CONSTANT_COLOR;
                case nxt::BlendFactor::OneMinusBlendColor:
                    return alpha ? GL_ONE_MINUS_CONSTANT_ALPHA : GL_ONE_MINUS_CONSTANT_COLOR;
                default:
                    UNREACHABLE();
            }
        }

        GLenum GLBlendMode(nxt::BlendOperation operation) {
            switch (operation) {
                case nxt::BlendOperation::Add:
                    return GL_FUNC_ADD;
                case nxt::BlendOperation::Subtract:
                    return GL_FUNC_SUBTRACT;
                case nxt::BlendOperation::ReverseSubtract:
                    return GL_FUNC_REVERSE_SUBTRACT;
                case nxt::BlendOperation::Min:
                    return GL_MIN;
                case nxt::BlendOperation::Max:
                    return GL_MAX;
                default:
                    UNREACHABLE();
            }
        }
    }  // namespace

    BlendState::BlendState(BlendStateBuilder* builder) : BlendStateBase(builder) {
    }

    void BlendState::ApplyNow(uint32_t attachment) {
        const auto& info = GetBlendInfo();

        if (info.blendEnabled) {
            glEnablei(GL_BLEND, attachment);
            glBlendEquationSeparatei(attachment, GLBlendMode(info.colorBlend.operation),
                                     GLBlendMode(info.alphaBlend.operation));
            glBlendFuncSeparatei(attachment, GLBlendFactor(info.colorBlend.srcFactor, false),
                                 GLBlendFactor(info.colorBlend.dstFactor, false),
                                 GLBlendFactor(info.alphaBlend.srcFactor, true),
                                 GLBlendFactor(info.alphaBlend.dstFactor, true));
        } else {
            glDisablei(GL_BLEND, attachment);
        }
        glColorMaski(attachment, info.colorWriteMask & nxt::ColorWriteMask::Red,
                     info.colorWriteMask & nxt::ColorWriteMask::Green,
                     info.colorWriteMask & nxt::ColorWriteMask::Blue,
                     info.colorWriteMask & nxt::ColorWriteMask::Alpha);
    }

}}  // namespace backend::opengl
