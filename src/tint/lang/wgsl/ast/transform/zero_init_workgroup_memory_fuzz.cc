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

#include "src/tint/lang/wgsl/ast/transform/zero_init_workgroup_memory.h"

#include "src/tint/cmd/fuzz/wgsl/wgsl_fuzz.h"
#include "src/tint/lang/wgsl/ast/module.h"

namespace tint::ast::transform {

void ZeroInitWorkgroupMemoryFuzzer(const tint::Program& program) {
    if (program.AST().HasOverrides()) {
        return;
    }

    DataMap outputs;
    if (auto result = ZeroInitWorkgroupMemory{}.Apply(program, DataMap{}, outputs)) {
        if (!result->IsValid()) {
            TINT_ICE() << "ZeroInitWorkgroupMemory returned invalid program:\n"
                       << result->Diagnostics();
        }
    }
}

}  // namespace tint::ast::transform

TINT_WGSL_PROGRAM_FUZZER(tint::ast::transform::ZeroInitWorkgroupMemoryFuzzer);
