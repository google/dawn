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

#include "src/tint/lang/spirv/writer/raise/raise.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/add_empty_entry_point.h"
#include "src/tint/lang/core/ir/transform/block_decorated_structs.h"
#include "src/tint/lang/core/ir/transform/demote_to_helper.h"
#include "src/tint/lang/core/ir/transform/std140.h"
#include "src/tint/lang/spirv/writer/raise/builtin_polyfill.h"
#include "src/tint/lang/spirv/writer/raise/expand_implicit_splats.h"
#include "src/tint/lang/spirv/writer/raise/handle_matrix_arithmetic.h"
#include "src/tint/lang/spirv/writer/raise/merge_return.h"
#include "src/tint/lang/spirv/writer/raise/shader_io.h"
#include "src/tint/lang/spirv/writer/raise/var_for_dynamic_index.h"

namespace tint::spirv::writer::raise {

Result<SuccessType, std::string> Raise(ir::Module* module) {
#define RUN_TRANSFORM(name)         \
    do {                            \
        auto result = name(module); \
        if (!result) {              \
            return result;          \
        }                           \
    } while (false)

    RUN_TRANSFORM(ir::transform::AddEmptyEntryPoint);
    RUN_TRANSFORM(ir::transform::BlockDecoratedStructs);
    RUN_TRANSFORM(BuiltinPolyfill);
    RUN_TRANSFORM(ir::transform::DemoteToHelper);
    RUN_TRANSFORM(ExpandImplicitSplats);
    RUN_TRANSFORM(HandleMatrixArithmetic);
    RUN_TRANSFORM(MergeReturn);
    RUN_TRANSFORM(ShaderIO);
    RUN_TRANSFORM(ir::transform::Std140);
    RUN_TRANSFORM(VarForDynamicIndex);

    return Success;
}

}  // namespace tint::spirv::writer::raise
