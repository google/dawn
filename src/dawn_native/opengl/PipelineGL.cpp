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
#include "common/Log.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Device.h"
#include "dawn_native/Pipeline.h"
#include "dawn_native/opengl/Forward.h"
#include "dawn_native/opengl/OpenGLFunctions.h"
#include "dawn_native/opengl/PipelineLayoutGL.h"
#include "dawn_native/opengl/SamplerGL.h"
#include "dawn_native/opengl/ShaderModuleGL.h"

#include <set>

namespace dawn_native { namespace opengl {

    namespace {

        GLenum GLShaderType(SingleShaderStage stage) {
            switch (stage) {
                case SingleShaderStage::Vertex:
                    return GL_VERTEX_SHADER;
                case SingleShaderStage::Fragment:
                    return GL_FRAGMENT_SHADER;
                case SingleShaderStage::Compute:
                    return GL_COMPUTE_SHADER;
            }
        }

    }  // namespace

    PipelineGL::PipelineGL() = default;
    PipelineGL::~PipelineGL() = default;

    void PipelineGL::Initialize(const OpenGLFunctions& gl,
                                const PipelineLayout* layout,
                                const PerStage<ProgrammableStage>& stages) {
        auto CreateShader = [](const OpenGLFunctions& gl, GLenum type,
                               const char* source) -> GLuint {
            GLuint shader = gl.CreateShader(type);
            gl.ShaderSource(shader, 1, &source, nullptr);
            gl.CompileShader(shader);

            GLint compileStatus = GL_FALSE;
            gl.GetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
            if (compileStatus == GL_FALSE) {
                GLint infoLogLength = 0;
                gl.GetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

                if (infoLogLength > 1) {
                    std::vector<char> buffer(infoLogLength);
                    gl.GetShaderInfoLog(shader, infoLogLength, nullptr, &buffer[0]);
                    dawn::ErrorLog() << source << "\nProgram compilation failed:\n"
                                     << buffer.data();
                }
            }
            return shader;
        };

        mProgram = gl.CreateProgram();

        // Compute the set of active stages.
        wgpu::ShaderStage activeStages = wgpu::ShaderStage::None;
        for (SingleShaderStage stage : IterateStages(kAllStages)) {
            if (stages[stage].module != nullptr) {
                activeStages |= StageBit(stage);
            }
        }

        // Create an OpenGL shader for each stage and gather the list of combined samplers.
        PerStage<CombinedSamplerInfo> combinedSamplers;
        bool needsDummySampler = false;
        for (SingleShaderStage stage : IterateStages(activeStages)) {
            const ShaderModule* module = ToBackend(stages[stage].module.Get());
            std::string glsl =
                module->TranslateToGLSL(stages[stage].entryPoint.c_str(), stage,
                                        &combinedSamplers[stage], layout, &needsDummySampler);
            GLuint shader = CreateShader(gl, GLShaderType(stage), glsl.c_str());
            gl.AttachShader(mProgram, shader);
        }

        if (needsDummySampler) {
            SamplerDescriptor desc = {};
            ASSERT(desc.minFilter == wgpu::FilterMode::Nearest);
            ASSERT(desc.magFilter == wgpu::FilterMode::Nearest);
            ASSERT(desc.mipmapFilter == wgpu::FilterMode::Nearest);
            mDummySampler = AcquireRef(
                ToBackend(layout->GetDevice()->GetOrCreateSampler(&desc).AcquireSuccess()));
        }

        // Link all the shaders together.
        gl.LinkProgram(mProgram);

        GLint linkStatus = GL_FALSE;
        gl.GetProgramiv(mProgram, GL_LINK_STATUS, &linkStatus);
        if (linkStatus == GL_FALSE) {
            GLint infoLogLength = 0;
            gl.GetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

            if (infoLogLength > 1) {
                std::vector<char> buffer(infoLogLength);
                gl.GetProgramInfoLog(mProgram, infoLogLength, nullptr, &buffer[0]);
                dawn::ErrorLog() << "Program link failed:\n" << buffer.data();
            }
        }

        // The uniforms are part of the program state so we can pre-bind buffer units, texture units
        // etc.
        gl.UseProgram(mProgram);
        const auto& indices = layout->GetBindingIndexInfo();

        for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
            const BindGroupLayoutBase* bgl = layout->GetBindGroupLayout(group);

            for (const auto& it : bgl->GetBindingMap()) {
                BindingNumber bindingNumber = it.first;
                BindingIndex bindingIndex = it.second;

                std::string name = GetBindingName(group, bindingNumber);
                const BindingInfo& bindingInfo = bgl->GetBindingInfo(bindingIndex);
                switch (bindingInfo.bindingType) {
                    case BindingInfoType::Buffer:
                        switch (bindingInfo.buffer.type) {
                            case wgpu::BufferBindingType::Uniform: {
                                GLint location = gl.GetUniformBlockIndex(mProgram, name.c_str());
                                if (location != -1) {
                                    gl.UniformBlockBinding(mProgram, location,
                                                           indices[group][bindingIndex]);
                                }
                                break;
                            }
                            case wgpu::BufferBindingType::Storage:
                            case wgpu::BufferBindingType::ReadOnlyStorage: {
                                // Since glShaderStorageBlockBinding doesn't exist in OpenGL ES, we
                                // skip that call and handle it during shader translation by
                                // modifying the location decoration. Contrary to all other binding
                                // types, OpenGL ES's SSBO binding index in the SSBO table is the
                                // value of the location= decoration in GLSL.
                                if (gl.GetVersion().IsDesktop()) {
                                    GLuint location = gl.GetProgramResourceIndex(
                                        mProgram, GL_SHADER_STORAGE_BLOCK, name.c_str());
                                    if (location != GL_INVALID_INDEX) {
                                        gl.ShaderStorageBlockBinding(mProgram, location,
                                                                     indices[group][bindingIndex]);
                                    }
                                }
                                break;
                            }
                            case wgpu::BufferBindingType::Undefined:
                                UNREACHABLE();
                        }
                        break;

                    case BindingInfoType::Sampler:
                    case BindingInfoType::Texture:
                        // These binding types are handled in the separate sampler and texture
                        // emulation
                        break;

                    case BindingInfoType::StorageTexture: {
                        GLint location = gl.GetUniformLocation(mProgram, name.c_str());
                        if (location != -1) {
                            gl.Uniform1i(location, indices[group][bindingIndex]);
                        }
                        break;
                    }
                }
            }
        }

        // Compute links between stages for combined samplers, then bind them to texture units
        {
            std::set<CombinedSampler> combinedSamplersSet;
            for (SingleShaderStage stage : IterateStages(activeStages)) {
                for (const CombinedSampler& combined : combinedSamplers[stage]) {
                    combinedSamplersSet.insert(combined);
                }
            }

            mUnitsForSamplers.resize(layout->GetNumSamplers());
            mUnitsForTextures.resize(layout->GetNumSampledTextures());

            GLuint textureUnit = layout->GetTextureUnitsUsed();
            for (const auto& combined : combinedSamplersSet) {
                std::string name = combined.GetName();
                GLint location = gl.GetUniformLocation(mProgram, name.c_str());

                if (location == -1) {
                    continue;
                }

                gl.Uniform1i(location, textureUnit);

                bool shouldUseFiltering;
                {
                    const BindGroupLayoutBase* bgl =
                        layout->GetBindGroupLayout(combined.textureLocation.group);
                    BindingIndex bindingIndex =
                        bgl->GetBindingIndex(combined.textureLocation.binding);

                    GLuint textureIndex = indices[combined.textureLocation.group][bindingIndex];
                    mUnitsForTextures[textureIndex].push_back(textureUnit);

                    shouldUseFiltering = bgl->GetBindingInfo(bindingIndex).texture.sampleType ==
                                         wgpu::TextureSampleType::Float;
                }
                {
                    if (combined.useDummySampler) {
                        mDummySamplerUnits.push_back(textureUnit);
                    } else {
                        const BindGroupLayoutBase* bgl =
                            layout->GetBindGroupLayout(combined.samplerLocation.group);
                        BindingIndex bindingIndex =
                            bgl->GetBindingIndex(combined.samplerLocation.binding);

                        GLuint samplerIndex = indices[combined.samplerLocation.group][bindingIndex];
                        mUnitsForSamplers[samplerIndex].push_back(
                            {textureUnit, shouldUseFiltering});
                    }
                }

                textureUnit++;
            }
        }
    }

    const std::vector<PipelineGL::SamplerUnit>& PipelineGL::GetTextureUnitsForSampler(
        GLuint index) const {
        ASSERT(index < mUnitsForSamplers.size());
        return mUnitsForSamplers[index];
    }

    const std::vector<GLuint>& PipelineGL::GetTextureUnitsForTextureView(GLuint index) const {
        ASSERT(index < mUnitsForTextures.size());
        return mUnitsForTextures[index];
    }

    GLuint PipelineGL::GetProgramHandle() const {
        return mProgram;
    }

    void PipelineGL::ApplyNow(const OpenGLFunctions& gl) {
        gl.UseProgram(mProgram);
        for (GLuint unit : mDummySamplerUnits) {
            ASSERT(mDummySampler.Get() != nullptr);
            gl.BindSampler(unit, mDummySampler->GetNonFilteringHandle());
        }
    }

}}  // namespace dawn_native::opengl
