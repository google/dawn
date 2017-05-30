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

#include "OpenGLBackend.h"

#include "CommandBufferGL.h"
#include "PipelineGL.h"
#include "PipelineLayoutGL.h"
#include "ShaderModuleGL.h"
#include "SamplerGL.h"
#include "TextureGL.h"

namespace backend {
namespace opengl {
    nxtProcTable GetNonValidatingProcs();
    nxtProcTable GetValidatingProcs();

    void HACKCLEAR() {
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearStencil(0);
		glClear(GL_STENCIL_BUFFER_BIT);
    }

    void Init(void* (*getProc)(const char*), nxtProcTable* procs, nxtDevice* device) {
        *device = nullptr;

        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(getProc));

        glEnable(GL_DEPTH_TEST);
        HACKCLEAR();

        *procs = GetValidatingProcs();
        *device = reinterpret_cast<nxtDevice>(new Device);
    }

	static GLuint OpenGLCompareFunction(nxt::CompareFunction compareFunction) {
		switch (compareFunction) {
			case nxt::CompareFunction::Never:
				return GL_NEVER;
			case nxt::CompareFunction::Less:
				return GL_LESS;
			case nxt::CompareFunction::LessEqual:
				return GL_LEQUAL;
			case nxt::CompareFunction::Greater:
				return GL_GREATER;
			case nxt::CompareFunction::GreaterEqual:
				return GL_GEQUAL;
			case nxt::CompareFunction::NotEqual:
				return GL_NOTEQUAL;
			case nxt::CompareFunction::Equal:
				return GL_EQUAL;
			case nxt::CompareFunction::Always:
				return GL_ALWAYS;
			default:
				ASSERT(false);
		}
	}

	static GLuint OpenGLStencilOperation(nxt::StencilOperation stencilOperation) {
		switch (stencilOperation) {
			case nxt::StencilOperation::Keep:
				return GL_KEEP;
			case nxt::StencilOperation::Zero:
				return GL_ZERO;
			case nxt::StencilOperation::Replace:
				return GL_REPLACE;
			case nxt::StencilOperation::Invert:
				return GL_INVERT;
			case nxt::StencilOperation::IncrementClamp:
				return GL_INCR;
			case nxt::StencilOperation::DecrementClamp:
				return GL_DECR;
			case nxt::StencilOperation::IncrementWrap:
				return GL_INCR_WRAP;
			case nxt::StencilOperation::DecrementWrap:
				return GL_DECR_WRAP;
			default: 
				ASSERT(false);
		}
	}

    // Device

    BindGroupBase* Device::CreateBindGroup(BindGroupBuilder* builder) {
        return new BindGroup(this, builder);
    }
    BindGroupLayoutBase* Device::CreateBindGroupLayout(BindGroupLayoutBuilder* builder) {
        return new BindGroupLayout(this, builder);
    }
    BufferBase* Device::CreateBuffer(BufferBuilder* builder) {
        return new Buffer(this, builder);
    }
    BufferViewBase* Device::CreateBufferView(BufferViewBuilder* builder) {
        return new BufferView(this, builder);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(this, builder);
    }
	DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
		return new DepthStencilState(this, builder);
	}
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(this, builder);
    }
    FramebufferBase* Device::CreateFramebuffer(FramebufferBuilder* builder) {
        return new Framebuffer(this, builder);
    }
    PipelineBase* Device::CreatePipeline(PipelineBuilder* builder) {
        return new Pipeline(this, builder);
    }
    PipelineLayoutBase* Device::CreatePipelineLayout(PipelineLayoutBuilder* builder) {
        return new PipelineLayout(this, builder);
    }
    QueueBase* Device::CreateQueue(QueueBuilder* builder) {
        return new Queue(this, builder);
    }
    RenderPassBase* Device::CreateRenderPass(RenderPassBuilder* builder) {
        return new RenderPass(this, builder);
    }
    SamplerBase* Device::CreateSampler(SamplerBuilder* builder) {
        return new Sampler(this, builder);
    }
    ShaderModuleBase* Device::CreateShaderModule(ShaderModuleBuilder* builder) {
        return new ShaderModule(this, builder);
    }
    TextureBase* Device::CreateTexture(TextureBuilder* builder) {
        return new Texture(this, builder);
    }
    TextureViewBase* Device::CreateTextureView(TextureViewBuilder* builder) {
        return new TextureView(this, builder);
    }

    void Device::Reference() {
    }

    void Device::Release() {
    }

    // Bind Group

    BindGroup::BindGroup(Device* device, BindGroupBuilder* builder)
        : BindGroupBase(builder), device(device) {
    }

    // Bind Group Layout

    BindGroupLayout::BindGroupLayout(Device* device, BindGroupLayoutBuilder* builder)
        : BindGroupLayoutBase(builder), device(device) {
    }

    // Buffer

    Buffer::Buffer(Device* device, BufferBuilder* builder)
        : BufferBase(builder), device(device) {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, GetSize(), nullptr, GL_STATIC_DRAW);
    }

    GLuint Buffer::GetHandle() const {
        return buffer;
    }

    void Buffer::SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) {
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferSubData(GL_ARRAY_BUFFER, start * sizeof(uint32_t), count * sizeof(uint32_t), data);
    }

    // BufferView

    BufferView::BufferView(Device* device, BufferViewBuilder* builder)
        : BufferViewBase(builder), device(device) {
    }

	DepthStencilState::DepthStencilState(Device* device, DepthStencilStateBuilder* builder)
		: DepthStencilStateBase(builder), device(device) {

	}

	void DepthStencilState::Apply() {
		if (DepthIsEnabled()) {
			glEnable(GL_DEPTH_TEST);
			auto& depth = GetDepth();
			glDepthFunc(OpenGLCompareFunction(depth.compareFunction));
			switch (depth.depthWriteMode) {
                case nxt::DepthWriteMode::Disabled:
                    glDepthMask(GL_FALSE);
                    break;
                case nxt::DepthWriteMode::Enabled:
                    glDepthMask(GL_TRUE);
                    break;
                default:
                    ASSERT(false);
                    break;
			}
		}
		else {
			glDisable(GL_DEPTH_TEST);
		}

		static const GLuint GL_FACES[2] = { GL_BACK, GL_FRONT };
		static const nxt::Face NXT_FACES[2] = { nxt::Face::Back, nxt::Face::Front };
		if (StencilIsEnabled()) {
			glEnable(GL_STENCIL_TEST);
			for (uint32_t i = 0; i < 2; ++i) {
				auto& stencil = GetStencil(NXT_FACES[i]);
				glStencilFuncSeparate(GL_FACES[i],
					OpenGLCompareFunction(stencil.compareFunction),
					stencil.reference,
					stencil.readMask
				);
				glStencilOpSeparate(GL_FACES[i],
					OpenGLStencilOperation(stencil.stencilFail),
					OpenGLStencilOperation(stencil.depthFail),
					OpenGLStencilOperation(stencil.stencilPass)
				);
				glStencilMaskSeparate(GL_FACES[i], stencil.writeMask);
			}
		}
		else {
			glDisable(GL_STENCIL_TEST);
		}
	}

    // InputState

    InputState::InputState(Device* device, InputStateBuilder* builder)
        : InputStateBase(builder), device(device) {
        glGenVertexArrays(1, &vertexArrayObject);
        glBindVertexArray(vertexArrayObject);
        auto& attributesSetMask = GetAttributesSetMask();
        for (uint32_t location = 0; location < attributesSetMask.size(); ++location) {
            if (!attributesSetMask[location]) {
                continue;
            }
            auto attribute = GetAttribute(location);
            glEnableVertexAttribArray(location);

            auto input = GetInput(attribute.bindingSlot);
            if (input.stride == 0) {
                // Emulate a stride of zero (constant vertex attribute) by
                // setting the attribute instance divisor to a huge number.
                glVertexAttribDivisor(location, 0xffffffff);
            } else {
                switch (input.stepMode) {
                    case nxt::InputStepMode::Vertex:
                        break;
                    case nxt::InputStepMode::Instance:
                        glVertexAttribDivisor(location, 1);
                        break;
                    default:
                        ASSERT(false);
                        break;
                }
            }
        }
    }

    GLuint InputState::GetVAO() {
        return vertexArrayObject;
    }

    // Framebuffer

    Framebuffer::Framebuffer(Device* device, FramebufferBuilder* builder)
        : FramebufferBase(builder), device(device) {
    }

    // Queue

    Queue::Queue(Device* device, QueueBuilder* builder)
        : QueueBase(builder), device(device) {
    }

    void Queue::Submit(uint32_t numCommands, CommandBuffer* const * commands) {
        for (uint32_t i = 0; i < numCommands; ++i) {
            commands[i]->Execute();
        }
    }

    // RenderPass

    RenderPass::RenderPass(Device* device, RenderPassBuilder* builder)
        : RenderPassBase(builder), device(device) {
    }

}
}
