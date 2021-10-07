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
#include "common/Log.h"
#include "dawn_native/TintUtils.h"
#include "dawn_native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn_native/d3d12/D3D12Error.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/PipelineLayoutD3D12.h"
#include "dawn_native/d3d12/PlatformFunctions.h"
#include "dawn_native/d3d12/UtilsD3D12.h"

#include <d3dcompiler.h>

#include <tint/tint.h>
#include <map>
#include <sstream>
#include <unordered_map>

namespace dawn_native { namespace d3d12 {

    namespace {
        ResultOrError<uint64_t> GetDXCompilerVersion(ComPtr<IDxcValidator> dxcValidator) {
            ComPtr<IDxcVersionInfo> versionInfo;
            DAWN_TRY(CheckHRESULT(dxcValidator.As(&versionInfo),
                                  "D3D12 QueryInterface IDxcValidator to IDxcVersionInfo"));

            uint32_t compilerMajor, compilerMinor;
            DAWN_TRY(CheckHRESULT(versionInfo->GetVersion(&compilerMajor, &compilerMinor),
                                  "IDxcVersionInfo::GetVersion"));

            // Pack both into a single version number.
            return (uint64_t(compilerMajor) << uint64_t(32)) + compilerMinor;
        }

        uint64_t GetD3DCompilerVersion() {
            return D3D_COMPILER_VERSION;
        }

        struct CompareBindingPoint {
            constexpr bool operator()(const tint::transform::BindingPoint& lhs,
                                      const tint::transform::BindingPoint& rhs) const {
                if (lhs.group != rhs.group) {
                    return lhs.group < rhs.group;
                } else {
                    return lhs.binding < rhs.binding;
                }
            }
        };

        void Serialize(std::stringstream& output, const tint::ast::Access& access) {
            output << access;
        }

        void Serialize(std::stringstream& output,
                       const tint::transform::BindingPoint& binding_point) {
            output << "(BindingPoint";
            output << " group=" << binding_point.group;
            output << " binding=" << binding_point.binding;
            output << ")";
        }

        template <typename T>
        void Serialize(std::stringstream& output,
                       const std::unordered_map<tint::transform::BindingPoint, T>& map) {
            output << "(map";

            std::map<tint::transform::BindingPoint, T, CompareBindingPoint> sorted(map.begin(),
                                                                                   map.end());
            for (auto& entry : sorted) {
                output << " ";
                Serialize(output, entry.first);
                output << "=";
                Serialize(output, entry.second);
            }
            output << ")";
        }

        // The inputs to a shader compilation. These have been intentionally isolated from the
        // device to help ensure that the pipeline cache key contains all inputs for compilation.
        struct ShaderCompilationRequest {
            enum Compiler { FXC, DXC };

            // Common inputs
            Compiler compiler;
            const tint::Program* program;
            const char* entryPointName;
            SingleShaderStage stage;
            uint32_t compileFlags;
            bool disableSymbolRenaming;
            tint::transform::BindingRemapper::BindingPoints bindingPoints;
            tint::transform::BindingRemapper::AccessControls accessControls;
            bool isRobustnessEnabled;

            // FXC/DXC common inputs
            bool disableWorkgroupInit;

            // FXC inputs
            uint64_t fxcVersion;

            // DXC inputs
            uint64_t dxcVersion;
            const D3D12DeviceInfo* deviceInfo;
            bool hasShaderFloat16Feature;

            static ResultOrError<ShaderCompilationRequest> Create(
                const char* entryPointName,
                SingleShaderStage stage,
                const PipelineLayout* layout,
                uint32_t compileFlags,
                const Device* device,
                const tint::Program* program,
                const BindingInfoArray& moduleBindingInfo) {
                Compiler compiler;
                uint64_t dxcVersion = 0;
                if (device->IsToggleEnabled(Toggle::UseDXC)) {
                    compiler = Compiler::DXC;
                    DAWN_TRY_ASSIGN(dxcVersion, GetDXCompilerVersion(device->GetDxcValidator()));
                } else {
                    compiler = Compiler::FXC;
                }

                using tint::transform::BindingPoint;
                using tint::transform::BindingRemapper;

                BindingRemapper::BindingPoints bindingPoints;
                BindingRemapper::AccessControls accessControls;

                // d3d12::BindGroupLayout packs the bindings per HLSL register-space. We modify the
                // Tint AST to make the "bindings" decoration match the offset chosen by
                // d3d12::BindGroupLayout so that Tint produces HLSL with the correct registers
                // assigned to each interface variable.
                for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
                    const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));
                    const auto& groupBindingInfo = moduleBindingInfo[group];
                    for (const auto& it : groupBindingInfo) {
                        BindingNumber binding = it.first;
                        auto const& bindingInfo = it.second;
                        BindingIndex bindingIndex = bgl->GetBindingIndex(binding);
                        BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                                     static_cast<uint32_t>(binding)};
                        BindingPoint dstBindingPoint{static_cast<uint32_t>(group),
                                                     bgl->GetShaderRegister(bindingIndex)};
                        if (srcBindingPoint != dstBindingPoint) {
                            bindingPoints.emplace(srcBindingPoint, dstBindingPoint);
                        }

                        // Declaring a read-only storage buffer in HLSL but specifying a storage
                        // buffer in the BGL produces the wrong output. Force read-only storage
                        // buffer bindings to be treated as UAV instead of SRV. Internal storage
                        // buffer is a storage buffer used in the internal pipeline.
                        const bool forceStorageBufferAsUAV =
                            (bindingInfo.buffer.type == wgpu::BufferBindingType::ReadOnlyStorage &&
                             (bgl->GetBindingInfo(bindingIndex).buffer.type ==
                                  wgpu::BufferBindingType::Storage ||
                              bgl->GetBindingInfo(bindingIndex).buffer.type ==
                                  kInternalStorageBufferBinding));
                        if (forceStorageBufferAsUAV) {
                            accessControls.emplace(srcBindingPoint, tint::ast::Access::kReadWrite);
                        }
                    }
                }

                ShaderCompilationRequest request;
                request.compiler = compiler;
                request.program = program;
                request.entryPointName = entryPointName;
                request.stage = stage;
                request.compileFlags = compileFlags;
                request.disableSymbolRenaming =
                    device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
                request.bindingPoints = std::move(bindingPoints);
                request.accessControls = std::move(accessControls);
                request.isRobustnessEnabled = device->IsRobustnessEnabled();
                request.disableWorkgroupInit =
                    device->IsToggleEnabled(Toggle::DisableWorkgroupInit);
                request.fxcVersion = compiler == Compiler::FXC ? GetD3DCompilerVersion() : 0;
                request.dxcVersion = compiler == Compiler::DXC ? dxcVersion : 0;
                request.deviceInfo = &device->GetDeviceInfo();
                request.hasShaderFloat16Feature = device->IsFeatureEnabled(Feature::ShaderFloat16);
                return std::move(request);
            }

            ResultOrError<PersistentCacheKey> CreateCacheKey() const {
                // Generate the WGSL from the Tint program so it's normalized.
                // TODO(tint:1180): Consider using a binary serialization of the tint AST for a more
                // compact representation.
                auto result = tint::writer::wgsl::Generate(program, tint::writer::wgsl::Options{});
                if (!result.success) {
                    std::ostringstream errorStream;
                    errorStream << "Tint WGSL failure:" << std::endl;
                    errorStream << "Generator: " << result.error << std::endl;
                    return DAWN_INTERNAL_ERROR(errorStream.str().c_str());
                }

                std::stringstream stream;

                // Prefix the key with the type to avoid collisions from another type that could
                // have the same key.
                stream << static_cast<uint32_t>(PersistentKeyType::Shader);
                stream << "\n";

                stream << result.wgsl.length();
                stream << "\n";

                stream << result.wgsl;
                stream << "\n";

                stream << "(ShaderCompilationRequest";
                stream << " compiler=" << compiler;
                stream << " entryPointName=" << entryPointName;
                stream << " stage=" << uint32_t(stage);
                stream << " compileFlags=" << compileFlags;
                stream << " disableSymbolRenaming=" << disableSymbolRenaming;

                stream << " bindingPoints=";
                Serialize(stream, bindingPoints);

                stream << " accessControls=";
                Serialize(stream, accessControls);

                stream << " shaderModel=" << deviceInfo->shaderModel;
                stream << " disableWorkgroupInit=" << disableWorkgroupInit;
                stream << " isRobustnessEnabled=" << isRobustnessEnabled;
                stream << " fxcVersion=" << fxcVersion;
                stream << " dxcVersion=" << dxcVersion;
                stream << " hasShaderFloat16Feature=" << hasShaderFloat16Feature;
                stream << ")";
                stream << "\n";

                return PersistentCacheKey(std::istreambuf_iterator<char>{stream},
                                          std::istreambuf_iterator<char>{});
            }
        };

        std::vector<const wchar_t*> GetDXCArguments(uint32_t compileFlags, bool enable16BitTypes) {
            std::vector<const wchar_t*> arguments;
            if (compileFlags & D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY) {
                arguments.push_back(L"/Gec");
            }
            if (compileFlags & D3DCOMPILE_IEEE_STRICTNESS) {
                arguments.push_back(L"/Gis");
            }
            if (compileFlags & D3DCOMPILE_OPTIMIZATION_LEVEL2) {
                switch (compileFlags & D3DCOMPILE_OPTIMIZATION_LEVEL2) {
                    case D3DCOMPILE_OPTIMIZATION_LEVEL0:
                        arguments.push_back(L"/O0");
                        break;
                    case D3DCOMPILE_OPTIMIZATION_LEVEL2:
                        arguments.push_back(L"/O2");
                        break;
                    case D3DCOMPILE_OPTIMIZATION_LEVEL3:
                        arguments.push_back(L"/O3");
                        break;
                }
            }
            if (compileFlags & D3DCOMPILE_DEBUG) {
                arguments.push_back(L"/Zi");
            }
            if (compileFlags & D3DCOMPILE_PACK_MATRIX_ROW_MAJOR) {
                arguments.push_back(L"/Zpr");
            }
            if (compileFlags & D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR) {
                arguments.push_back(L"/Zpc");
            }
            if (compileFlags & D3DCOMPILE_AVOID_FLOW_CONTROL) {
                arguments.push_back(L"/Gfa");
            }
            if (compileFlags & D3DCOMPILE_PREFER_FLOW_CONTROL) {
                arguments.push_back(L"/Gfp");
            }
            if (compileFlags & D3DCOMPILE_RESOURCES_MAY_ALIAS) {
                arguments.push_back(L"/res_may_alias");
            }

            if (enable16BitTypes) {
                // enable-16bit-types are only allowed in -HV 2018 (default)
                arguments.push_back(L"/enable-16bit-types");
            }

            arguments.push_back(L"-HV");
            arguments.push_back(L"2018");

            return arguments;
        }

        ResultOrError<ComPtr<IDxcBlob>> CompileShaderDXC(IDxcLibrary* dxcLibrary,
                                                         IDxcCompiler* dxcCompiler,
                                                         const ShaderCompilationRequest& request,
                                                         const std::string& hlslSource) {
            ComPtr<IDxcBlobEncoding> sourceBlob;
            DAWN_TRY(
                CheckHRESULT(dxcLibrary->CreateBlobWithEncodingOnHeapCopy(
                                 hlslSource.c_str(), hlslSource.length(), CP_UTF8, &sourceBlob),
                             "DXC create blob"));

            std::wstring entryPointW;
            DAWN_TRY_ASSIGN(entryPointW, ConvertStringToWstring(request.entryPointName));

            std::vector<const wchar_t*> arguments =
                GetDXCArguments(request.compileFlags, request.hasShaderFloat16Feature);

            ComPtr<IDxcOperationResult> result;
            DAWN_TRY(CheckHRESULT(
                dxcCompiler->Compile(sourceBlob.Get(), nullptr, entryPointW.c_str(),
                                     request.deviceInfo->shaderProfiles[request.stage].c_str(),
                                     arguments.data(), arguments.size(), nullptr, 0, nullptr,
                                     &result),
                "DXC compile"));

            HRESULT hr;
            DAWN_TRY(CheckHRESULT(result->GetStatus(&hr), "DXC get status"));

            if (FAILED(hr)) {
                ComPtr<IDxcBlobEncoding> errors;
                DAWN_TRY(CheckHRESULT(result->GetErrorBuffer(&errors), "DXC get error buffer"));

                std::string message = std::string("DXC compile failed with ") +
                                      static_cast<char*>(errors->GetBufferPointer());
                return DAWN_VALIDATION_ERROR(message);
            }

            ComPtr<IDxcBlob> compiledShader;
            DAWN_TRY(CheckHRESULT(result->GetResult(&compiledShader), "DXC get result"));
            return std::move(compiledShader);
        }

        ResultOrError<ComPtr<ID3DBlob>> CompileShaderFXC(const PlatformFunctions* functions,
                                                         const ShaderCompilationRequest& request,
                                                         const std::string& hlslSource) {
            const char* targetProfile = nullptr;
            switch (request.stage) {
                case SingleShaderStage::Vertex:
                    targetProfile = "vs_5_1";
                    break;
                case SingleShaderStage::Fragment:
                    targetProfile = "ps_5_1";
                    break;
                case SingleShaderStage::Compute:
                    targetProfile = "cs_5_1";
                    break;
            }

            ComPtr<ID3DBlob> compiledShader;
            ComPtr<ID3DBlob> errors;

            if (FAILED(functions->d3dCompile(hlslSource.c_str(), hlslSource.length(), nullptr,
                                             nullptr, nullptr, request.entryPointName,
                                             targetProfile, request.compileFlags, 0,
                                             &compiledShader, &errors))) {
                std::string message = std::string("D3D compile failed with ") +
                                      static_cast<char*>(errors->GetBufferPointer());
                return DAWN_VALIDATION_ERROR(message);
            }

            return std::move(compiledShader);
        }

        ResultOrError<std::string> TranslateToHLSL(const ShaderCompilationRequest& request,
                                                   std::string* remappedEntryPointName) {
            std::ostringstream errorStream;
            errorStream << "Tint HLSL failure:" << std::endl;

            tint::transform::Manager transformManager;
            tint::transform::DataMap transformInputs;

            if (request.isRobustnessEnabled) {
                transformManager.Add<tint::transform::Robustness>();
            }
            transformManager.Add<tint::transform::BindingRemapper>();

            transformManager.Add<tint::transform::Renamer>();

            if (request.disableSymbolRenaming) {
                // We still need to rename HLSL reserved keywords
                transformInputs.Add<tint::transform::Renamer::Config>(
                    tint::transform::Renamer::Target::kHlslKeywords);
            }

            // D3D12 registers like `t3` and `c3` have the same bindingOffset number in
            // the remapping but should not be considered a collision because they have
            // different types.
            const bool mayCollide = true;
            transformInputs.Add<tint::transform::BindingRemapper::Remappings>(
                std::move(request.bindingPoints), std::move(request.accessControls), mayCollide);

            tint::Program transformedProgram;
            tint::transform::DataMap transformOutputs;
            DAWN_TRY_ASSIGN(transformedProgram,
                            RunTransforms(&transformManager, request.program, transformInputs,
                                          &transformOutputs, nullptr));

            if (auto* data = transformOutputs.Get<tint::transform::Renamer::Data>()) {
                auto it = data->remappings.find(request.entryPointName);
                if (it != data->remappings.end()) {
                    *remappedEntryPointName = it->second;
                } else {
                    if (request.disableSymbolRenaming) {
                        *remappedEntryPointName = request.entryPointName;
                    } else {
                        return DAWN_VALIDATION_ERROR(
                            "Could not find remapped name for entry point.");
                    }
                }
            } else {
                return DAWN_VALIDATION_ERROR("Transform output missing renamer data.");
            }

            tint::writer::hlsl::Options options;
            options.disable_workgroup_init = request.disableWorkgroupInit;
            auto result = tint::writer::hlsl::Generate(&transformedProgram, options);
            if (!result.success) {
                errorStream << "Generator: " << result.error << std::endl;
                return DAWN_VALIDATION_ERROR(errorStream.str().c_str());
            }

            return std::move(result.hlsl);
        }

        template <typename F>
        MaybeError CompileShader(const PlatformFunctions* functions,
                                 IDxcLibrary* dxcLibrary,
                                 IDxcCompiler* dxcCompiler,
                                 ShaderCompilationRequest&& request,
                                 bool dumpShaders,
                                 F&& DumpShadersEmitLog,
                                 CompiledShader* compiledShader) {
            // Compile the source shader to HLSL.
            std::string hlslSource;
            std::string remappedEntryPoint;
            DAWN_TRY_ASSIGN(hlslSource, TranslateToHLSL(request, &remappedEntryPoint));
            if (dumpShaders) {
                std::ostringstream dumpedMsg;
                dumpedMsg << "/* Dumped generated HLSL */" << std::endl << hlslSource;
                DumpShadersEmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
            }
            request.entryPointName = remappedEntryPoint.c_str();
            switch (request.compiler) {
                case ShaderCompilationRequest::Compiler::DXC:
                    DAWN_TRY_ASSIGN(compiledShader->compiledDXCShader,
                                    CompileShaderDXC(dxcLibrary, dxcCompiler, request, hlslSource));
                    break;
                case ShaderCompilationRequest::Compiler::FXC:
                    DAWN_TRY_ASSIGN(compiledShader->compiledFXCShader,
                                    CompileShaderFXC(functions, request, hlslSource));
                    break;
            }

            return {};
        }

    }  // anonymous namespace

    // static
    ResultOrError<Ref<ShaderModule>> ShaderModule::Create(Device* device,
                                                          const ShaderModuleDescriptor* descriptor,
                                                          ShaderModuleParseResult* parseResult) {
        Ref<ShaderModule> module = AcquireRef(new ShaderModule(device, descriptor));
        DAWN_TRY(module->Initialize(parseResult));
        return module;
    }

    ShaderModule::ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor)
        : ShaderModuleBase(device, descriptor) {
    }

    MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult) {
        ScopedTintICEHandler scopedICEHandler(GetDevice());
        return InitializeBase(parseResult);
    }

    ResultOrError<CompiledShader> ShaderModule::Compile(const char* entryPointName,
                                                        SingleShaderStage stage,
                                                        PipelineLayout* layout,
                                                        uint32_t compileFlags) {
        ASSERT(!IsError());
        ScopedTintICEHandler scopedICEHandler(GetDevice());

        Device* device = ToBackend(GetDevice());

        CompiledShader compiledShader = {};

        tint::transform::Manager transformManager;
        tint::transform::DataMap transformInputs;

        const tint::Program* program;
        tint::Program programAsValue;
        if (stage == SingleShaderStage::Vertex) {
            transformManager.Add<tint::transform::FirstIndexOffset>();
            transformInputs.Add<tint::transform::FirstIndexOffset::BindingPoint>(
                layout->GetFirstIndexOffsetShaderRegister(),
                layout->GetFirstIndexOffsetRegisterSpace());

            tint::transform::DataMap transformOutputs;
            DAWN_TRY_ASSIGN(programAsValue,
                            RunTransforms(&transformManager, GetTintProgram(), transformInputs,
                                          &transformOutputs, nullptr));

            if (auto* data = transformOutputs.Get<tint::transform::FirstIndexOffset::Data>()) {
                // TODO(dawn:549): Consider adding this information to the pipeline cache once we
                // can store more than the shader blob in it.
                compiledShader.firstOffsetInfo.usesVertexIndex = data->has_vertex_index;
                if (compiledShader.firstOffsetInfo.usesVertexIndex) {
                    compiledShader.firstOffsetInfo.vertexIndexOffset = data->first_vertex_offset;
                }
                compiledShader.firstOffsetInfo.usesInstanceIndex = data->has_instance_index;
                if (compiledShader.firstOffsetInfo.usesInstanceIndex) {
                    compiledShader.firstOffsetInfo.instanceIndexOffset =
                        data->first_instance_offset;
                }
            }

            program = &programAsValue;
        } else {
            program = GetTintProgram();
        }

        ShaderCompilationRequest request;
        DAWN_TRY_ASSIGN(request, ShaderCompilationRequest::Create(
                                     entryPointName, stage, layout, compileFlags, device, program,
                                     GetEntryPoint(entryPointName).bindings));

        PersistentCacheKey shaderCacheKey;
        DAWN_TRY_ASSIGN(shaderCacheKey, request.CreateCacheKey());

        DAWN_TRY_ASSIGN(
            compiledShader.cachedShader,
            device->GetPersistentCache()->GetOrCreate(
                shaderCacheKey, [&](auto doCache) -> MaybeError {
                    DAWN_TRY(CompileShader(
                        device->GetFunctions(),
                        device->IsToggleEnabled(Toggle::UseDXC) ? device->GetDxcLibrary().Get()
                                                                : nullptr,
                        device->IsToggleEnabled(Toggle::UseDXC) ? device->GetDxcCompiler().Get()
                                                                : nullptr,
                        std::move(request), device->IsToggleEnabled(Toggle::DumpShaders),
                        [&](WGPULoggingType loggingType, const char* message) {
                            GetDevice()->EmitLog(loggingType, message);
                        },
                        &compiledShader));
                    const D3D12_SHADER_BYTECODE shader = compiledShader.GetD3D12ShaderBytecode();
                    doCache(shader.pShaderBytecode, shader.BytecodeLength);
                    return {};
                }));

        return std::move(compiledShader);
    }

    D3D12_SHADER_BYTECODE CompiledShader::GetD3D12ShaderBytecode() const {
        if (cachedShader.buffer != nullptr) {
            return {cachedShader.buffer.get(), cachedShader.bufferSize};
        } else if (compiledFXCShader != nullptr) {
            return {compiledFXCShader->GetBufferPointer(), compiledFXCShader->GetBufferSize()};
        } else if (compiledDXCShader != nullptr) {
            return {compiledDXCShader->GetBufferPointer(), compiledDXCShader->GetBufferSize()};
        }
        UNREACHABLE();
        return {};
    }
}}  // namespace dawn_native::d3d12
