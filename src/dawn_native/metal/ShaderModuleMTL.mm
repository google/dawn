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

#include "dawn_native/metal/ShaderModuleMTL.h"

#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/SpirvUtils.h"
#include "dawn_native/metal/DeviceMTL.h"
#include "dawn_native/metal/PipelineLayoutMTL.h"
#include "dawn_native/metal/RenderPipelineMTL.h"

#include <spirv_msl.hpp>

#ifdef DAWN_ENABLE_WGSL
// Tint include must be after spirv_msl.hpp, because spirv-cross has its own
// version of spirv_headers. We also need to undef SPV_REVISION because SPIRV-Cross
// is at 3 while spirv-headers is at 4.
#    undef SPV_REVISION
#    include <tint/tint.h>
#endif  // DAWN_ENABLE_WGSL

#include <sstream>

namespace dawn_native { namespace metal {

    // static
    ResultOrError<ShaderModule*> ShaderModule::Create(Device* device,
                                                      const ShaderModuleDescriptor* descriptor,
                                                      ShaderModuleParseResult* parseResult) {
        Ref<ShaderModule> module = AcquireRef(new ShaderModule(device, descriptor));
        DAWN_TRY(module->Initialize(parseResult));
        return module.Detach();
    }

    ShaderModule::ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor)
        : ShaderModuleBase(device, descriptor) {
    }

    MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult) {
        DAWN_TRY(InitializeBase(parseResult));
#ifdef DAWN_ENABLE_WGSL
        mTintModule = std::move(parseResult->tintModule);
#endif
        return {};
    }

    ResultOrError<std::string> ShaderModule::TranslateToMSLWithTint(
        const char* entryPointName,
        SingleShaderStage stage,
        const PipelineLayout* layout,
        // TODO(crbug.com/tint/387): AND in a fixed sample mask in the shader.
        uint32_t sampleMask,
        const RenderPipeline* renderPipeline,
        std::string* remappedEntryPointName,
        bool* needsStorageBufferLength) {
#if DAWN_ENABLE_WGSL
        // TODO(crbug.com/tint/256): Set this accordingly if arrayLength(..) is used.
        *needsStorageBufferLength = false;

        std::ostringstream errorStream;
        errorStream << "Tint MSL failure:" << std::endl;

        tint::transform::Manager transformManager;
        if (stage == SingleShaderStage::Vertex &&
            GetDevice()->IsToggleEnabled(Toggle::MetalEnableVertexPulling)) {
            transformManager.append(
                MakeVertexPullingTransform(*renderPipeline->GetVertexStateDescriptor(),
                                           entryPointName, kPullingBufferBindingSet));

            for (VertexBufferSlot slot :
                 IterateBitSet(renderPipeline->GetVertexBufferSlotsUsed())) {
                uint32_t metalIndex = renderPipeline->GetMtlVertexBufferIndex(slot);
                DAWN_UNUSED(metalIndex);
                // TODO(crbug.com/tint/104): Tell Tint to map (kPullingBufferBindingSet, slot) to
                // this MSL buffer index.
            }
        }
        transformManager.append(std::make_unique<tint::transform::BoundArrayAccessors>());

        tint::ast::Module module;
        DAWN_TRY_ASSIGN(module, RunTransforms(&transformManager, mTintModule.get()));

        ASSERT(remappedEntryPointName != nullptr);
        tint::inspector::Inspector inspector(module);
        *remappedEntryPointName = inspector.GetRemappedNameForEntryPoint(entryPointName);

        tint::writer::msl::Generator generator(std::move(module));
        if (!generator.Generate()) {
            errorStream << "Generator: " << generator.error() << std::endl;
            return DAWN_VALIDATION_ERROR(errorStream.str().c_str());
        }

        std::string msl = generator.result();
        return std::move(msl);
#else
        UNREACHABLE();
#endif
    }

    ResultOrError<std::string> ShaderModule::TranslateToMSLWithSPIRVCross(
        const char* entryPointName,
        SingleShaderStage stage,
        const PipelineLayout* layout,
        uint32_t sampleMask,
        const RenderPipeline* renderPipeline,
        std::string* remappedEntryPointName,
        bool* needsStorageBufferLength) {
        const std::vector<uint32_t>* spirv = &GetSpirv();
        spv::ExecutionModel executionModel = ShaderStageToExecutionModel(stage);

#ifdef DAWN_ENABLE_WGSL
        std::vector<uint32_t> pullingSpirv;
        if (GetDevice()->IsToggleEnabled(Toggle::MetalEnableVertexPulling) &&
            stage == SingleShaderStage::Vertex) {
            if (mTintModule) {
                DAWN_TRY_ASSIGN(pullingSpirv,
                                GeneratePullingSpirv(mTintModule.get(),
                                                     *renderPipeline->GetVertexStateDescriptor(),
                                                     entryPointName, kPullingBufferBindingSet));
            } else {
                DAWN_TRY_ASSIGN(
                    pullingSpirv,
                    GeneratePullingSpirv(GetSpirv(), *renderPipeline->GetVertexStateDescriptor(),
                                         entryPointName, kPullingBufferBindingSet));
            }
            spirv = &pullingSpirv;
        }
#endif

        // If these options are changed, the values in DawnSPIRVCrossMSLFastFuzzer.cpp need to
        // be updated.
        spirv_cross::CompilerMSL::Options options_msl;

        // Disable PointSize builtin for https://bugs.chromium.org/p/dawn/issues/detail?id=146
        // Because Metal will reject PointSize builtin if the shader is compiled into a render
        // pipeline that uses a non-point topology.
        // TODO (hao.x.li@intel.com): Remove this once WebGPU requires there is no
        // gl_PointSize builtin (https://github.com/gpuweb/gpuweb/issues/332).
        options_msl.enable_point_size_builtin = false;

        // Always use vertex buffer 30 (the last one in the vertex buffer table) to contain
        // the shader storage buffer lengths.
        options_msl.buffer_size_buffer_index = kBufferLengthBufferSlot;

        options_msl.additional_fixed_sample_mask = sampleMask;

        spirv_cross::CompilerMSL compiler(*spirv);
        compiler.set_msl_options(options_msl);
        compiler.set_entry_point(entryPointName, executionModel);

        // By default SPIRV-Cross will give MSL resources indices in increasing order.
        // To make the MSL indices match the indices chosen in the PipelineLayout, we build
        // a table of MSLResourceBinding to give to SPIRV-Cross.

        // Create one resource binding entry per stage per binding.
        for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
            const BindGroupLayoutBase::BindingMap& bindingMap =
                layout->GetBindGroupLayout(group)->GetBindingMap();

            for (const auto& it : bindingMap) {
                BindingNumber bindingNumber = it.first;
                BindingIndex bindingIndex = it.second;

                const BindingInfo& bindingInfo =
                    layout->GetBindGroupLayout(group)->GetBindingInfo(bindingIndex);

                if (!(bindingInfo.visibility & StageBit(stage))) {
                    continue;
                }

                uint32_t shaderIndex = layout->GetBindingIndexInfo(stage)[group][bindingIndex];

                spirv_cross::MSLResourceBinding mslBinding;
                mslBinding.stage = executionModel;
                mslBinding.desc_set = static_cast<uint32_t>(group);
                mslBinding.binding = static_cast<uint32_t>(bindingNumber);
                mslBinding.msl_buffer = mslBinding.msl_texture = mslBinding.msl_sampler =
                    shaderIndex;

                compiler.add_msl_resource_binding(mslBinding);
            }
        }

#ifdef DAWN_ENABLE_WGSL
        // Add vertex buffers bound as storage buffers
        if (GetDevice()->IsToggleEnabled(Toggle::MetalEnableVertexPulling) &&
            stage == SingleShaderStage::Vertex) {
            for (VertexBufferSlot slot :
                 IterateBitSet(renderPipeline->GetVertexBufferSlotsUsed())) {
                uint32_t metalIndex = renderPipeline->GetMtlVertexBufferIndex(slot);

                spirv_cross::MSLResourceBinding mslBinding;

                mslBinding.stage = spv::ExecutionModelVertex;
                mslBinding.desc_set = static_cast<uint32_t>(kPullingBufferBindingSet);
                mslBinding.binding = static_cast<uint8_t>(slot);
                mslBinding.msl_buffer = metalIndex;
                compiler.add_msl_resource_binding(mslBinding);
            }
        }
#endif

        // SPIRV-Cross also supports re-ordering attributes but it seems to do the correct thing
        // by default.
        std::string msl = compiler.compile();

        // Some entry point names are forbidden in MSL so SPIRV-Cross modifies them. Query the
        // modified entryPointName from it.
        *remappedEntryPointName = compiler.get_entry_point(entryPointName, executionModel).name;
        *needsStorageBufferLength = compiler.needs_buffer_size_buffer();

        return std::move(msl);
    }

    MaybeError ShaderModule::CreateFunction(const char* entryPointName,
                                            SingleShaderStage stage,
                                            const PipelineLayout* layout,
                                            ShaderModule::MetalFunctionData* out,
                                            uint32_t sampleMask,
                                            const RenderPipeline* renderPipeline) {
        ASSERT(!IsError());
        ASSERT(out);

        std::string remappedEntryPointName;
        std::string msl;
        if (GetDevice()->IsToggleEnabled(Toggle::UseTintGenerator)) {
            DAWN_TRY_ASSIGN(msl, TranslateToMSLWithTint(entryPointName, stage, layout, sampleMask,
                                                        renderPipeline, &remappedEntryPointName,
                                                        &out->needsStorageBufferLength));
        } else {
            DAWN_TRY_ASSIGN(msl, TranslateToMSLWithSPIRVCross(
                                     entryPointName, stage, layout, sampleMask, renderPipeline,
                                     &remappedEntryPointName, &out->needsStorageBufferLength));
        }

        // Metal uses Clang to compile the shader as C++14. Disable everything in the -Wall
        // category. -Wunused-variable in particular comes up a lot in generated code, and some
        // (old?) Metal drivers accidentally treat it as a MTLLibraryErrorCompileError instead
        // of a warning.
        msl = R"(\
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
)" + msl;

        NSRef<NSString> mslSource = AcquireNSRef([[NSString alloc] initWithUTF8String:msl.c_str()]);

        auto mtlDevice = ToBackend(GetDevice())->GetMTLDevice();
        NSError* error = nullptr;
        NSPRef<id<MTLLibrary>> library =
            AcquireNSPRef([mtlDevice newLibraryWithSource:mslSource.Get()
                                                  options:nullptr
                                                    error:&error]);
        if (error != nullptr) {
            if (error.code != MTLLibraryErrorCompileWarning) {
                const char* errorString = [error.localizedDescription UTF8String];
                return DAWN_VALIDATION_ERROR(std::string("Unable to create library object: ") +
                                             errorString);
            }
        }

        NSRef<NSString> name =
            AcquireNSRef([[NSString alloc] initWithUTF8String:remappedEntryPointName.c_str()]);
        out->function = AcquireNSPRef([*library newFunctionWithName:name.Get()]);

        if (GetDevice()->IsToggleEnabled(Toggle::MetalEnableVertexPulling) &&
            GetEntryPoint(entryPointName).usedVertexAttributes.any()) {
            out->needsStorageBufferLength = true;
        }

        return {};
    }
}}  // namespace dawn_native::metal
