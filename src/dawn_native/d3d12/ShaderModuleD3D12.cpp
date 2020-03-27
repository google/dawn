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

#include "dawn_native/d3d12/ShaderModuleD3D12.h"

#include "common/Assert.h"
#include "common/BitSetIterator.h"
#include "dawn_native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/PipelineLayoutD3D12.h"

#include <spirv_hlsl.hpp>

namespace dawn_native { namespace d3d12 {

    // static
    ResultOrError<ShaderModule*> ShaderModule::Create(Device* device,
                                                      const ShaderModuleDescriptor* descriptor) {
        std::unique_ptr<ShaderModule> module(new ShaderModule(device, descriptor));
        if (!module)
            return DAWN_VALIDATION_ERROR("Unable to create ShaderModule");
        DAWN_TRY(module->Initialize(descriptor));
        return module.release();
    }

    ShaderModule::ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor)
        : ShaderModuleBase(device, descriptor) {
    }

    MaybeError ShaderModule::Initialize(const ShaderModuleDescriptor* descriptor) {
        mSpirv.assign(descriptor->code, descriptor->code + descriptor->codeSize);
        if (GetDevice()->IsToggleEnabled(Toggle::UseSpvc)) {
            shaderc_spvc::CompileOptions options = GetCompileOptions();

            options.SetHLSLShaderModel(51);
            // PointCoord and PointSize are not supported in HLSL
            // TODO (hao.x.li@intel.com): The point_coord_compat and point_size_compat are
            // required temporarily for https://bugs.chromium.org/p/dawn/issues/detail?id=146,
            // but should be removed once WebGPU requires there is no gl_PointSize builtin.
            // See https://github.com/gpuweb/gpuweb/issues/332
            options.SetHLSLPointCoordCompat(true);
            options.SetHLSLPointSizeCompat(true);

            DAWN_TRY(CheckSpvcSuccess(
                mSpvcContext.InitializeForHlsl(descriptor->code, descriptor->codeSize, options),
                "Unable to initialize instance of spvc"));

            spirv_cross::Compiler* compiler;
            DAWN_TRY(CheckSpvcSuccess(mSpvcContext.GetCompiler(reinterpret_cast<void**>(&compiler)),
                                      "Unable to get cross compiler"));
            DAWN_TRY(ExtractSpirvInfo(*compiler));
        } else {
            spirv_cross::CompilerHLSL compiler(descriptor->code, descriptor->codeSize);
            DAWN_TRY(ExtractSpirvInfo(compiler));
        }
        return {};
    }

    ResultOrError<std::string> ShaderModule::GetHLSLSource(PipelineLayout* layout) {
        std::unique_ptr<spirv_cross::CompilerHLSL> compiler_impl;
        spirv_cross::CompilerHLSL* compiler;
        if (!GetDevice()->IsToggleEnabled(Toggle::UseSpvc)) {
            // If these options are changed, the values in DawnSPIRVCrossHLSLFastFuzzer.cpp need to
            // be updated.
            spirv_cross::CompilerGLSL::Options options_glsl;

            spirv_cross::CompilerHLSL::Options options_hlsl;
            options_hlsl.shader_model = 51;
            // PointCoord and PointSize are not supported in HLSL
            // TODO (hao.x.li@intel.com): The point_coord_compat and point_size_compat are
            // required temporarily for https://bugs.chromium.org/p/dawn/issues/detail?id=146,
            // but should be removed once WebGPU requires there is no gl_PointSize builtin.
            // See https://github.com/gpuweb/gpuweb/issues/332
            options_hlsl.point_coord_compat = true;
            options_hlsl.point_size_compat = true;

            compiler_impl = std::make_unique<spirv_cross::CompilerHLSL>(mSpirv);
            compiler = compiler_impl.get();
            compiler->set_common_options(options_glsl);
            compiler->set_hlsl_options(options_hlsl);
        }

        const ModuleBindingInfo& moduleBindingInfo = GetBindingInfo();
        for (uint32_t group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
            const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));
            const auto& bindingOffsets = bgl->GetBindingOffsets();
            const auto& groupBindingInfo = moduleBindingInfo[group];
            for (const auto& it : groupBindingInfo) {
                const ShaderBindingInfo& bindingInfo = it.second;
                BindingNumber bindingNumber = it.first;
                BindingIndex bindingIndex = bgl->GetBindingIndex(bindingNumber);

                uint32_t bindingOffset = bindingOffsets[bindingIndex];
                if (GetDevice()->IsToggleEnabled(Toggle::UseSpvc)) {
                    DAWN_TRY(CheckSpvcSuccess(
                        mSpvcContext.SetDecoration(bindingInfo.id, SHADERC_SPVC_DECORATION_BINDING,
                                                   bindingOffset),
                        "Unable to set decorating binding before generating HLSL shader w/ "
                        "spvc"));
                } else {
                    compiler->set_decoration(bindingInfo.id, spv::DecorationBinding, bindingOffset);
                }
            }
        }
        if (GetDevice()->IsToggleEnabled(Toggle::UseSpvc)) {
            shaderc_spvc::CompilationResult result;
            DAWN_TRY(CheckSpvcSuccess(mSpvcContext.CompileShader(&result),
                                      "Unable to generate HLSL shader w/ spvc"));
            std::string result_string;
            DAWN_TRY(CheckSpvcSuccess(result.GetStringOutput(&result_string),
                                      "Unable to get HLSL shader text"));
            return result_string;
        } else {
            return compiler->compile();
        }
    }

}}  // namespace dawn_native::d3d12
