// Copyright 2023 The Tint Authors.
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

#include "src/tint/api/tint.h"

////////////////////////////////////////////////////////////////////////////////
// The following includes are used by './tools/run gen' to add an implicit
// dependency from 'tint/api' to the libraries used to make up the Tint API.
////////////////////////////////////////////////////////////////////////////////
#include "src/tint/api/common/override_id.h"
#include "src/tint/api/options/array_length_from_uniform.h"

#if TINT_BUILD_GLSL_WRITER
#include "src/tint/lang/glsl/writer/writer.h"  // nogncheck
#endif

#if TINT_BUILD_HLSL_WRITER
#include "src/tint/lang/hlsl/writer/writer.h"  // nogncheck
#endif

#if TINT_BUILD_MSL_WRITER
#include "src/tint/lang/msl/writer/writer.h"  // nogncheck
#endif

#if TINT_BUILD_SPV_READER
#include "src/tint/lang/spirv/reader/reader.h"  // nogncheck
#endif

#if TINT_BUILD_SPV_WRITER
#include "src/tint/lang/spirv/writer/writer.h"  // nogncheck
#endif

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/reader/reader.h"  // nogncheck
#endif

#if TINT_BUILD_WGSL_WRITER
#include "src/tint/lang/wgsl/writer/writer.h"  // nogncheck
#endif

namespace tint {

/// Initialize initializes the Tint library. Call before using the Tint API.
void Initialize() {
#if TINT_BUILD_WGSL_WRITER
    // Register the Program printer. This is used for debugging purposes.
    tint::Program::printer = [](const tint::Program& program) {
        auto result = wgsl::writer::Generate(program, {});
        if (!result) {
            return result.Failure().reason.str();
        }
        return result->wgsl;
    };
#endif
}

/// Shutdown uninitializes the Tint library. Call after using the Tint API.
void Shutdown() {
    // Currently no-op, but may release tint resources in the future.
}

}  // namespace tint
