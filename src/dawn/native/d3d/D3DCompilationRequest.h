// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D_D3DCOMPILATIONREQUEST_H_
#define SRC_DAWN_NATIVE_D3D_D3DCOMPILATIONREQUEST_H_

#include <d3dcompiler.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "dawn/native/CacheRequest.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/d3d/d3d_platform.h"

#include "tint/tint.h"

namespace dawn::native::stream {

// Define no-op serializations for pD3DCompile, IDxcLibrary, and IDxcCompiler3.
// These are output-only interfaces used to generate bytecode.
template <>
inline void Stream<IDxcLibrary*>::Write(Sink*, IDxcLibrary* const&) {}
template <>
inline void Stream<IDxcCompiler3*>::Write(Sink*, IDxcCompiler3* const&) {}
template <>
inline void Stream<pD3DCompile>::Write(Sink*, pD3DCompile const&) {}

}  // namespace dawn::native::stream

namespace dawn::native::d3d {

enum class Compiler { FXC, DXC };

using AccessControl = std::unordered_map<tint::BindingPoint, tint::core::Access>;

using InterStageShaderVariablesMask = std::bitset<tint::hlsl::writer::kMaxInterStageLocations>;

#define HLSL_COMPILATION_REQUEST_MEMBERS(X)                                                      \
    X(const tint::Program*, inputProgram)                                                        \
    X(std::string_view, entryPointName)                                                          \
    X(SingleShaderStage, stage)                                                                  \
    X(uint32_t, shaderModel)                                                                     \
    X(uint32_t, compileFlags)                                                                    \
    X(Compiler, compiler)                                                                        \
    X(uint64_t, compilerVersion)                                                                 \
    X(std::wstring_view, dxcShaderProfile)                                                       \
    X(std::string_view, fxcShaderProfile)                                                        \
    X(pD3DCompile, d3dCompile)                                                                   \
    X(IDxcLibrary*, dxcLibrary)                                                                  \
    X(IDxcCompiler3*, dxcCompiler)                                                               \
    X(uint32_t, firstIndexOffsetShaderRegister)                                                  \
    X(uint32_t, firstIndexOffsetRegisterSpace)                                                   \
    X(bool, usesNumWorkgroups)                                                                   \
    X(uint32_t, numWorkgroupsShaderRegister)                                                     \
    X(uint32_t, numWorkgroupsRegisterSpace)                                                      \
    X(tint::ExternalTextureOptions, externalTextureOptions)                                      \
    X(tint::ArrayLengthFromUniformOptions, arrayLengthFromUniform)                               \
    X(tint::BindingRemapperOptions, bindingRemapper)                                             \
    X(AccessControl, accessControls)                                                             \
    X(std::optional<tint::ast::transform::SubstituteOverride::Config>, substituteOverrideConfig) \
    X(InterStageShaderVariablesMask, interstageLocations)                                        \
    X(LimitsForCompilationRequest, limits)                                                       \
    X(bool, disableSymbolRenaming)                                                               \
    X(bool, isRobustnessEnabled)                                                                 \
    X(bool, disableWorkgroupInit)                                                                \
    X(bool, polyfillReflectVec2F32)                                                              \
    X(bool, dumpShaders)                                                                         \
    X(std::vector<tint::BindingPoint>, bindingPointsIgnoredInRobustnessTransform)

#define D3D_BYTECODE_COMPILATION_REQUEST_MEMBERS(X) \
    X(bool, hasShaderF16Feature)                    \
    X(uint32_t, compileFlags)                       \
    X(Compiler, compiler)                           \
    X(uint64_t, compilerVersion)                    \
    X(std::wstring_view, dxcShaderProfile)          \
    X(std::string_view, fxcShaderProfile)           \
    X(pD3DCompile, d3dCompile)                      \
    X(IDxcLibrary*, dxcLibrary)                     \
    X(IDxcCompiler3*, dxcCompiler)

DAWN_SERIALIZABLE(struct, HlslCompilationRequest, HLSL_COMPILATION_REQUEST_MEMBERS){};
#undef HLSL_COMPILATION_REQUEST_MEMBERS

DAWN_SERIALIZABLE(struct,
                  D3DBytecodeCompilationRequest,
                  D3D_BYTECODE_COMPILATION_REQUEST_MEMBERS){};
#undef D3D_BYTECODE_COMPILATION_REQUEST_MEMBERS

#define D3D_COMPILATION_REQUEST_MEMBERS(X)     \
    X(HlslCompilationRequest, hlsl)            \
    X(D3DBytecodeCompilationRequest, bytecode) \
    X(CacheKey::UnsafeUnkeyedValue<dawn::platform::Platform*>, tracePlatform)

DAWN_MAKE_CACHE_REQUEST(D3DCompilationRequest, D3D_COMPILATION_REQUEST_MEMBERS);
#undef D3D_COMPILATION_REQUEST_MEMBERS

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_D3DCOMPILATIONREQUEST_H_
