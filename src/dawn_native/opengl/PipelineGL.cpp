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

#include "dawn_native/opengl/PipelineGL.h"

#include "common/BitSetIterator.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/opengl/Forward.h"
#include "dawn_native/opengl/PersistentPipelineStateGL.h"
#include "dawn_native/opengl/PipelineLayoutGL.h"
#include "dawn_native/opengl/ShaderModuleGL.h"

#include <iostream>
#include <set>

namespace dawn_native { namespace opengl {

    namespace {

        GLenum GLShaderType(dawn::ShaderStage stage) {
            switch (stage) {
                case dawn::ShaderStage::Vertex:
                    return GL_VERTEX_SHADER;
                case dawn::ShaderStage::Fragment:
                    return GL_FRAGMENT_SHADER;
                case dawn::ShaderStage::Compute:
                    return GL_COMPUTE_SHADER;
                default:
                    UNREACHABLE();
            }
        }

    }  // namespace

    PipelineGL::PipelineGL() {
    }

    void PipelineGL::Initialize(const PipelineLayout* layout,
                                const PerStage<const ShaderModule*>& modules) {
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

        auto FillPushConstants = [](const ShaderModule* module, GLPushConstantInfo* info,
                                    GLuint program) {
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

        mProgram = glCreateProgram();

        dawn::ShaderStageBit activeStages = dawn::ShaderStageBit::None;
        for (dawn::ShaderStage stage : IterateStages(kAllStages)) {
            if (modules[stage] != nullptr) {
                activeStages |= StageBit(stage);
            }
        }

        for (dawn::ShaderStage stage : IterateStages(activeStages)) {
            GLuint shader = CreateShader(GLShaderType(stage), modules[stage]->GetSource());
            glAttachShader(mProgram, shader);
        }

        glLinkProgram(mProgram);

        GLint linkStatus = GL_FALSE;
        glGetProgramiv(mProgram, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE) {
            GLint infoLogLength = 0;
            glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

            if (infoLogLength > 1) {
                std::vector<char> buffer(infoLogLength);
                glGetProgramInfoLog(mProgram, infoLogLength, nullptr, &buffer[0]);
                std::cout << "Program link failed:\n";
                std::cout << buffer.data() << std::endl;
            }
        }

        for (dawn::ShaderStage stage : IterateStages(activeStages)) {
            FillPushConstants(modules[stage], &mGlPushConstants[stage], mProgram);
        }

        glUseProgram(mProgram);

        // The uniforms are part of the program state so we can pre-bind buffer units, texture units
        // etc.
        const auto& indices = layout->GetBindingIndexInfo();

        for (uint32_t group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
            const auto& groupInfo = layout->GetBindGroupLayout(group)->GetBindingInfo();

            for (uint32_t binding = 0; binding < kMaxBindingsPerGroup; ++binding) {
                if (!groupInfo.mask[binding]) {
                    continue;
                }

                std::string name = GetBindingName(group, binding);
                switch (groupInfo.types[binding]) {
                    case dawn::BindingType::UniformBuffer: {
                        GLint location = glGetUniformBlockIndex(mProgram, name.c_str());
                        glUniformBlockBinding(mProgram, location, indices[group][binding]);
                    } break;

                    case dawn::BindingType::StorageBuffer: {
                        GLuint location = glGetProgramResourceIndex(
                            mProgram, GL_SHADER_STORAGE_BLOCK, name.c_str());
                        glShaderStorageBlockBinding(mProgram, location, indices[group][binding]);
                    } break;

                    case dawn::BindingType::Sampler:
                    case dawn::BindingType::SampledTexture:
                        // These binding types are handled in the separate sampler and texture
                        // emulation
                        break;
                }
            }
        }

        // Compute links between stages for combined samplers, then bind them to texture units
        {
            std::set<CombinedSampler> combinedSamplersSet;
            for (dawn::ShaderStage stage : IterateStages(activeStages)) {
                for (const auto& combined : modules[stage]->GetCombinedSamplerInfo()) {
                    combinedSamplersSet.insert(combined);
                }
            }

            mUnitsForSamplers.resize(layout->GetNumSamplers());
            mUnitsForTextures.resize(layout->GetNumSampledTextures());

            GLuint textureUnit = layout->GetTextureUnitsUsed();
            for (const auto& combined : combinedSamplersSet) {
                std::string name = combined.GetName();
                GLint location = glGetUniformLocation(mProgram, name.c_str());
                glUniform1i(location, textureUnit);

                GLuint samplerIndex =
                    indices[combined.samplerLocation.group][combined.samplerLocation.binding];
                mUnitsForSamplers[samplerIndex].push_back(textureUnit);

                GLuint textureIndex =
                    indices[combined.textureLocation.group][combined.textureLocation.binding];
                mUnitsForTextures[textureIndex].push_back(textureUnit);

                textureUnit++;
            }
        }
    }

    const PipelineGL::GLPushConstantInfo& PipelineGL::GetGLPushConstants(
        dawn::ShaderStage stage) const {
        return mGlPushConstants[stage];
    }

    const std::vector<GLuint>& PipelineGL::GetTextureUnitsForSampler(GLuint index) const {
        ASSERT(index < mUnitsForSamplers.size());
        return mUnitsForSamplers[index];
    }

    const std::vector<GLuint>& PipelineGL::GetTextureUnitsForTextureView(GLuint index) const {
        ASSERT(index < mUnitsForSamplers.size());
        return mUnitsForTextures[index];
    }

    GLuint PipelineGL::GetProgramHandle() const {
        return mProgram;
    }

    void PipelineGL::ApplyNow() {
        glUseProgram(mProgram);
    }

}}  // namespace dawn_native::opengl
