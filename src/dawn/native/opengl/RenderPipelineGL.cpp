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

#include "dawn/native/opengl/RenderPipelineGL.h"

#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/Forward.h"
#include "dawn/native/opengl/PersistentPipelineStateGL.h"
#include "dawn/native/opengl/UtilsGL.h"

namespace dawn::native::opengl {

namespace {

GLenum GLPrimitiveTopology(wgpu::PrimitiveTopology primitiveTopology) {
    switch (primitiveTopology) {
        case wgpu::PrimitiveTopology::PointList:
            return GL_POINTS;
        case wgpu::PrimitiveTopology::LineList:
            return GL_LINES;
        case wgpu::PrimitiveTopology::LineStrip:
            return GL_LINE_STRIP;
        case wgpu::PrimitiveTopology::TriangleList:
            return GL_TRIANGLES;
        case wgpu::PrimitiveTopology::TriangleStrip:
            return GL_TRIANGLE_STRIP;
    }
    UNREACHABLE();
}

void ApplyFrontFaceAndCulling(const OpenGLFunctions& gl,
                              wgpu::FrontFace face,
                              wgpu::CullMode mode) {
    // Note that we invert winding direction in OpenGL. Because Y axis is up in OpenGL,
    // which is different from WebGPU and other backends (Y axis is down).
    GLenum direction = (face == wgpu::FrontFace::CCW) ? GL_CW : GL_CCW;
    gl.FrontFace(direction);

    if (mode == wgpu::CullMode::None) {
        gl.Disable(GL_CULL_FACE);
    } else {
        gl.Enable(GL_CULL_FACE);

        GLenum cullMode = (mode == wgpu::CullMode::Front) ? GL_FRONT : GL_BACK;
        gl.CullFace(cullMode);
    }
}

GLenum GLBlendFactor(wgpu::BlendFactor factor, bool alpha) {
    switch (factor) {
        case wgpu::BlendFactor::Zero:
            return GL_ZERO;
        case wgpu::BlendFactor::One:
            return GL_ONE;
        case wgpu::BlendFactor::Src:
            return GL_SRC_COLOR;
        case wgpu::BlendFactor::OneMinusSrc:
            return GL_ONE_MINUS_SRC_COLOR;
        case wgpu::BlendFactor::SrcAlpha:
            return GL_SRC_ALPHA;
        case wgpu::BlendFactor::OneMinusSrcAlpha:
            return GL_ONE_MINUS_SRC_ALPHA;
        case wgpu::BlendFactor::Dst:
            return GL_DST_COLOR;
        case wgpu::BlendFactor::OneMinusDst:
            return GL_ONE_MINUS_DST_COLOR;
        case wgpu::BlendFactor::DstAlpha:
            return GL_DST_ALPHA;
        case wgpu::BlendFactor::OneMinusDstAlpha:
            return GL_ONE_MINUS_DST_ALPHA;
        case wgpu::BlendFactor::SrcAlphaSaturated:
            return GL_SRC_ALPHA_SATURATE;
        case wgpu::BlendFactor::Constant:
            return alpha ? GL_CONSTANT_ALPHA : GL_CONSTANT_COLOR;
        case wgpu::BlendFactor::OneMinusConstant:
            return alpha ? GL_ONE_MINUS_CONSTANT_ALPHA : GL_ONE_MINUS_CONSTANT_COLOR;
    }
    UNREACHABLE();
}

GLenum GLBlendMode(wgpu::BlendOperation operation) {
    switch (operation) {
        case wgpu::BlendOperation::Add:
            return GL_FUNC_ADD;
        case wgpu::BlendOperation::Subtract:
            return GL_FUNC_SUBTRACT;
        case wgpu::BlendOperation::ReverseSubtract:
            return GL_FUNC_REVERSE_SUBTRACT;
        case wgpu::BlendOperation::Min:
            return GL_MIN;
        case wgpu::BlendOperation::Max:
            return GL_MAX;
    }
    UNREACHABLE();
}

void ApplyColorState(const OpenGLFunctions& gl,
                     ColorAttachmentIndex attachment,
                     const ColorTargetState* state) {
    GLuint colorBuffer = static_cast<GLuint>(static_cast<uint8_t>(attachment));
    if (state->blend != nullptr) {
        gl.Enablei(GL_BLEND, colorBuffer);
        gl.BlendEquationSeparatei(colorBuffer, GLBlendMode(state->blend->color.operation),
                                  GLBlendMode(state->blend->alpha.operation));
        gl.BlendFuncSeparatei(colorBuffer, GLBlendFactor(state->blend->color.srcFactor, false),
                              GLBlendFactor(state->blend->color.dstFactor, false),
                              GLBlendFactor(state->blend->alpha.srcFactor, true),
                              GLBlendFactor(state->blend->alpha.dstFactor, true));
    } else {
        gl.Disablei(GL_BLEND, colorBuffer);
    }
    gl.ColorMaski(colorBuffer, state->writeMask & wgpu::ColorWriteMask::Red,
                  state->writeMask & wgpu::ColorWriteMask::Green,
                  state->writeMask & wgpu::ColorWriteMask::Blue,
                  state->writeMask & wgpu::ColorWriteMask::Alpha);
}

void ApplyColorState(const OpenGLFunctions& gl, const ColorTargetState* state) {
    if (state->blend != nullptr) {
        gl.Enable(GL_BLEND);
        gl.BlendEquationSeparate(GLBlendMode(state->blend->color.operation),
                                 GLBlendMode(state->blend->alpha.operation));
        gl.BlendFuncSeparate(GLBlendFactor(state->blend->color.srcFactor, false),
                             GLBlendFactor(state->blend->color.dstFactor, false),
                             GLBlendFactor(state->blend->alpha.srcFactor, true),
                             GLBlendFactor(state->blend->alpha.dstFactor, true));
    } else {
        gl.Disable(GL_BLEND);
    }
    gl.ColorMask(state->writeMask & wgpu::ColorWriteMask::Red,
                 state->writeMask & wgpu::ColorWriteMask::Green,
                 state->writeMask & wgpu::ColorWriteMask::Blue,
                 state->writeMask & wgpu::ColorWriteMask::Alpha);
}

bool Equal(const BlendComponent& lhs, const BlendComponent& rhs) {
    return lhs.operation == rhs.operation && lhs.srcFactor == rhs.srcFactor &&
           lhs.dstFactor == rhs.dstFactor;
}

GLuint OpenGLStencilOperation(wgpu::StencilOperation stencilOperation) {
    switch (stencilOperation) {
        case wgpu::StencilOperation::Keep:
            return GL_KEEP;
        case wgpu::StencilOperation::Zero:
            return GL_ZERO;
        case wgpu::StencilOperation::Replace:
            return GL_REPLACE;
        case wgpu::StencilOperation::Invert:
            return GL_INVERT;
        case wgpu::StencilOperation::IncrementClamp:
            return GL_INCR;
        case wgpu::StencilOperation::DecrementClamp:
            return GL_DECR;
        case wgpu::StencilOperation::IncrementWrap:
            return GL_INCR_WRAP;
        case wgpu::StencilOperation::DecrementWrap:
            return GL_DECR_WRAP;
    }
    UNREACHABLE();
}

void ApplyDepthStencilState(const OpenGLFunctions& gl,
                            const DepthStencilState* descriptor,
                            PersistentPipelineState* persistentPipelineState) {
    // Depth writes only occur if depth is enabled
    if (descriptor->depthCompare == wgpu::CompareFunction::Always &&
        !descriptor->depthWriteEnabled) {
        gl.Disable(GL_DEPTH_TEST);
    } else {
        gl.Enable(GL_DEPTH_TEST);
    }

    if (descriptor->depthWriteEnabled) {
        gl.DepthMask(GL_TRUE);
    } else {
        gl.DepthMask(GL_FALSE);
    }

    gl.DepthFunc(ToOpenGLCompareFunction(descriptor->depthCompare));

    if (StencilTestEnabled(descriptor)) {
        gl.Enable(GL_STENCIL_TEST);
    } else {
        gl.Disable(GL_STENCIL_TEST);
    }

    GLenum backCompareFunction = ToOpenGLCompareFunction(descriptor->stencilBack.compare);
    GLenum frontCompareFunction = ToOpenGLCompareFunction(descriptor->stencilFront.compare);
    persistentPipelineState->SetStencilFuncsAndMask(gl, backCompareFunction, frontCompareFunction,
                                                    descriptor->stencilReadMask);

    gl.StencilOpSeparate(GL_BACK, OpenGLStencilOperation(descriptor->stencilBack.failOp),
                         OpenGLStencilOperation(descriptor->stencilBack.depthFailOp),
                         OpenGLStencilOperation(descriptor->stencilBack.passOp));
    gl.StencilOpSeparate(GL_FRONT, OpenGLStencilOperation(descriptor->stencilFront.failOp),
                         OpenGLStencilOperation(descriptor->stencilFront.depthFailOp),
                         OpenGLStencilOperation(descriptor->stencilFront.passOp));

    gl.StencilMask(descriptor->stencilWriteMask);
}

}  // anonymous namespace

// static
Ref<RenderPipeline> RenderPipeline::CreateUninitialized(
    Device* device,
    const RenderPipelineDescriptor* descriptor) {
    return AcquireRef(new RenderPipeline(device, descriptor));
}

RenderPipeline::RenderPipeline(Device* device, const RenderPipelineDescriptor* descriptor)
    : RenderPipelineBase(device, descriptor),
      mVertexArrayObject(0),
      mGlPrimitiveTopology(GLPrimitiveTopology(GetPrimitiveTopology())) {}

MaybeError RenderPipeline::Initialize() {
    DAWN_TRY(
        InitializeBase(ToBackend(GetDevice())->GetGL(), ToBackend(GetLayout()), GetAllStages()));
    CreateVAOForVertexState();
    return {};
}

RenderPipeline::~RenderPipeline() = default;

void RenderPipeline::DestroyImpl() {
    RenderPipelineBase::DestroyImpl();
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
    gl.DeleteVertexArrays(1, &mVertexArrayObject);
    gl.BindVertexArray(0);
    DeleteProgram(gl);
}

GLenum RenderPipeline::GetGLPrimitiveTopology() const {
    return mGlPrimitiveTopology;
}

ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes>
RenderPipeline::GetAttributesUsingVertexBuffer(VertexBufferSlot slot) const {
    ASSERT(!IsError());
    return mAttributesUsingVertexBuffer[slot];
}

void RenderPipeline::CreateVAOForVertexState() {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();

    gl.GenVertexArrays(1, &mVertexArrayObject);
    gl.BindVertexArray(mVertexArrayObject);

    for (VertexAttributeLocation location : IterateBitSet(GetAttributeLocationsUsed())) {
        const auto& attribute = GetAttribute(location);
        GLuint glAttrib = static_cast<GLuint>(static_cast<uint8_t>(location));
        gl.EnableVertexAttribArray(glAttrib);

        mAttributesUsingVertexBuffer[attribute.vertexBufferSlot][location] = true;
        const VertexBufferInfo& vertexBuffer = GetVertexBuffer(attribute.vertexBufferSlot);

        if (vertexBuffer.arrayStride == 0) {
            // Emulate a stride of zero (constant vertex attribute) by
            // setting the attribute instance divisor to a huge number.
            gl.VertexAttribDivisor(glAttrib, 0xffffffff);
        } else {
            switch (vertexBuffer.stepMode) {
                case wgpu::VertexStepMode::Vertex:
                    break;
                case wgpu::VertexStepMode::Instance:
                    gl.VertexAttribDivisor(glAttrib, 1);
                    break;
                case wgpu::VertexStepMode::VertexBufferNotUsed:
                    UNREACHABLE();
            }
        }
    }
}

void RenderPipeline::ApplyNow(PersistentPipelineState& persistentPipelineState) {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
    PipelineGL::ApplyNow(gl);

    ASSERT(mVertexArrayObject);
    gl.BindVertexArray(mVertexArrayObject);

    ApplyFrontFaceAndCulling(gl, GetFrontFace(), GetCullMode());

    ApplyDepthStencilState(gl, GetDepthStencilState(), &persistentPipelineState);

    gl.SampleMaski(0, GetSampleMask());
    if (IsAlphaToCoverageEnabled()) {
        gl.Enable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    } else {
        gl.Disable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    }

    if (IsDepthBiasEnabled()) {
        gl.Enable(GL_POLYGON_OFFSET_FILL);
        float depthBias = GetDepthBias();
        float slopeScale = GetDepthBiasSlopeScale();
        if (gl.PolygonOffsetClamp != nullptr) {
            gl.PolygonOffsetClamp(slopeScale, depthBias, GetDepthBiasClamp());
        } else {
            gl.PolygonOffset(slopeScale, depthBias);
        }
    } else {
        gl.Disable(GL_POLYGON_OFFSET_FILL);
    }

    if (!GetDevice()->IsToggleEnabled(Toggle::DisableIndexedDrawBuffers)) {
        for (ColorAttachmentIndex attachmentSlot : IterateBitSet(GetColorAttachmentsMask())) {
            ApplyColorState(gl, attachmentSlot, GetColorTargetState(attachmentSlot));
        }
    } else {
        const ColorTargetState* prevDescriptor = nullptr;
        for (ColorAttachmentIndex attachmentSlot : IterateBitSet(GetColorAttachmentsMask())) {
            const ColorTargetState* descriptor = GetColorTargetState(attachmentSlot);
            if (!prevDescriptor) {
                ApplyColorState(gl, descriptor);
                prevDescriptor = descriptor;
            } else if ((descriptor->blend == nullptr) != (prevDescriptor->blend == nullptr)) {
                // TODO(crbug.com/dawn/582): GLES < 3.2 does not support different blend states
                // per color target. Add validation to prevent this as it is not.
                ASSERT(false);
            } else if (descriptor->blend != nullptr) {
                if (!Equal(descriptor->blend->alpha, prevDescriptor->blend->alpha) ||
                    !Equal(descriptor->blend->color, prevDescriptor->blend->color) ||
                    descriptor->writeMask != prevDescriptor->writeMask) {
                    // TODO(crbug.com/dawn/582)
                    ASSERT(false);
                }
            }
        }
    }
}

}  // namespace dawn::native::opengl
