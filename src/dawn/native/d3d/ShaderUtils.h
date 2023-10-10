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

#ifndef SRC_DAWN_NATIVE_D3D_SHADEUTILS_H_
#define SRC_DAWN_NATIVE_D3D_SHADEUTILS_H_

#include <string>

#include "dawn/native/Blob.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/d3d/D3DCompilationRequest.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d {

class Device;

#define COMPILED_SHADER_MEMBERS(X) \
    X(Blob, shaderBlob)            \
    X(std::string, hlslSource)     \
    X(bool, usesVertexIndex)       \
    X(bool, usesInstanceIndex)

// `CompiledShader` holds a ref to one of the various representations of shader blobs and
// information used to emulate vertex/instance index starts. It also holds the `hlslSource` for the
// shader compilation, which is only transiently available during Compile, and cleared before it
// returns. It is not written to or loaded from the cache unless Toggle dump_shaders is true.
DAWN_SERIALIZABLE(struct, CompiledShader, COMPILED_SHADER_MEMBERS){};
#undef COMPILED_SHADER_MEMBERS

std::string CompileFlagsToString(uint32_t compileFlags);

ResultOrError<CompiledShader> CompileShader(d3d::D3DCompilationRequest r);

void DumpFXCCompiledShader(Device* device,
                           const CompiledShader& compiledShader,
                           uint32_t compileFlags);

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_SHADEUTILS_H_
