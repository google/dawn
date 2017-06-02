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

#include "PipelineGL.h"

#include "DepthStencilStateGL.h"
#include "OpenGLBackend.h"
#include "PersistentPipelineStateGL.h"
#include "PipelineLayoutGL.h"
#include "ShaderModuleGL.h"

#include <iostream>
#include <set>

namespace backend {
namespace opengl {

    namespace {

        GLenum GLShaderType(nxt::ShaderStage stage) {
            switch (stage) {
                case nxt::ShaderStage::Vertex:
                    return GL_VERTEX_SHADER;
                case nxt::ShaderStage::Fragment:
                    return GL_FRAGMENT_SHADER;
                case nxt::ShaderStage::Compute:
                    return GL_COMPUTE_SHADER;
            }
        }

    }

    Pipeline::Pipeline(Device* device, PipelineBuilder* builder) : PipelineBase(builder), device(device) {
        auto CreateShader = [](GLenum type, const char* source) -> GLuint {
            GLuint shader = glCreateShader(type);
            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);

            GLint compileStatus = GL_FALSE;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
            if (compileStatus == GL_FALSE) {
                GLint infoLogLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

                if (infoLogLength > 1) {
                    std::vector<char> buffer(infoLogLength);
                    glGetShaderInfoLog(shader, infoLogLength, nullptr, &buffer[0]);
                    std::cout << source << std::endl;
                    std::cout << "Program compilation failed:\n";
                    std::cout << buffer.data() << std::endl;
                }
            }
            return shader;
        };

        auto FillPushConstants = [](const ShaderModule* module, GLPushConstantInfo* info, GLuint program) {
            const auto& moduleInfo = module->GetPushConstants();
            for (uint32_t i = 0; i < moduleInfo.names.size(); i++) {
                (*info)[i] = -1;

                unsigned int size = moduleInfo.sizes[i];
                if (size == 0) {
                    continue;
                }

                GLint location = glGetUniformLocation(program, moduleInfo.names[i].c_str());
                if (location == -1) {
                    continue;
                }

                for (uint32_t offset = 0; offset < size; offset++) {
                    (*info)[i + offset] = location + offset;
                }
                i += size - 1;
            }
        };

        program = glCreateProgram();

        for (auto stage : IterateStages(GetStageMask())) {
            const ShaderModule* module = ToBackend(builder->GetStageInfo(stage).module.Get());

            GLuint shader = CreateShader(GLShaderType(stage), module->GetSource());
            glAttachShader(program, shader);
        }

        glLinkProgram(program);

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE) {
            GLint infoLogLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

            if (infoLogLength > 1) {
                std::vector<char> buffer(infoLogLength);
                glGetProgramInfoLog(program, infoLogLength, nullptr, &buffer[0]);
                std::cout << "Program link failed:\n";
                std::cout << buffer.data() << std::endl;
            }
        }

        for (auto stage : IterateStages(GetStageMask())) {
            const ShaderModule* module = ToBackend(builder->GetStageInfo(stage).module.Get());
            FillPushConstants(module, &glPushConstants[stage], program);
        }

        glUseProgram(program);

        // The uniforms are part of the program state so we can pre-bind buffer units, texture units etc.
        const auto& layout = ToBackend(GetLayout());
        const auto& indices = layout->GetBindingIndexInfo();

        for (uint32_t group = 0; group < kMaxBindGroups; ++group) {
            const auto& groupInfo = layout->GetBindGroupLayout(group)->GetBindingInfo();

            for (uint32_t binding = 0; binding < kMaxBindingsPerGroup; ++binding) {
                if (!groupInfo.mask[binding]) {
                    continue;
                }

                std::string name = GetBindingName(group, binding);
                switch (groupInfo.types[binding]) {
                    case nxt::BindingType::UniformBuffer:
                        {
                            GLint location = glGetUniformBlockIndex(program, name.c_str());
                            glUniformBlockBinding(program, location, indices[group][binding]);
                        }
                        break;

                    case nxt::BindingType::StorageBuffer:
                        {
                            GLuint location = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, name.c_str());
                            glShaderStorageBlockBinding(program, location, indices[group][binding]);
                        }
                        break;

                    case nxt::BindingType::Sampler:
                    case nxt::BindingType::SampledTexture:
                        // These binding types are handled in the separate sampler and texture emulation
                        break;

                }
            }
        }

        // Compute links between stages for combined samplers, then bind them to texture units
        {
            std::set<CombinedSampler> combinedSamplersSet;
            for (auto stage : IterateStages(GetStageMask())) {
                const auto& module = ToBackend(builder->GetStageInfo(stage).module);

                for (const auto& combined : module->GetCombinedSamplerInfo()) {
                    combinedSamplersSet.insert(combined);
                }
            }

            unitsForSamplers.resize(layout->GetNumSamplers());
            unitsForTextures.resize(layout->GetNumSampledTextures());

            GLuint textureUnit = layout->GetTextureUnitsUsed();
            for (const auto& combined : combinedSamplersSet) {
                std::string name = combined.GetName();
                GLint location = glGetUniformLocation(program, name.c_str());
                glUniform1i(location, textureUnit);

                GLuint samplerIndex = indices[combined.samplerLocation.group][combined.samplerLocation.binding];
                unitsForSamplers[samplerIndex].push_back(textureUnit);

                GLuint textureIndex = indices[combined.textureLocation.group][combined.textureLocation.binding];
                unitsForTextures[textureIndex].push_back(textureUnit);

                textureUnit ++;
            }
        }
    }

    const Pipeline::GLPushConstantInfo& Pipeline::GetGLPushConstants(nxt::ShaderStage stage) const {
        return glPushConstants[stage];
    }

    const std::vector<GLuint>& Pipeline::GetTextureUnitsForSampler(GLuint index) const {
        ASSERT(index >= 0 && index < unitsForSamplers.size());
        return unitsForSamplers[index];
    }

    const std::vector<GLuint>& Pipeline::GetTextureUnitsForTexture(GLuint index) const {
        ASSERT(index >= 0 && index < unitsForSamplers.size());
        return unitsForTextures[index];
    }

    GLuint Pipeline::GetProgramHandle() const {
        return program;
    }

    void Pipeline::ApplyNow(PersistentPipelineState &persistentPipelineState) {
        glUseProgram(program);

        auto inputState = ToBackend(GetInputState());
        glBindVertexArray(inputState->GetVAO());

        auto depthStencilState = ToBackend(GetDepthStencilState());
        depthStencilState->ApplyNow(persistentPipelineState);
    }

}
}
