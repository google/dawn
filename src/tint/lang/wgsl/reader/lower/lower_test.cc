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

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/wgsl/builtin_fn.h"
#include "src/tint/lang/wgsl/ir/builtin_call.h"
#include "src/tint/lang/wgsl/reader/lower/lower.h"

namespace tint::wgsl::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using Wgslreader_LowerTest = core::ir::transform::TransformTest;

TEST_F(Wgslreader_LowerTest, BuiltinConversion) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {  //
        auto* result = b.InstructionResult(ty.i32());
        b.Append(b.ir.instructions.Create<wgsl::ir::BuiltinCall>(result, wgsl::BuiltinFn::kMax,
                                                                 Vector{
                                                                     b.Value(i32(1)),
                                                                     b.Value(i32(2)),
                                                                 }));
        b.Return(f);
    });

    auto* src = R"(
%f = func():void -> %b1 {
  %b1 = block {
    %2:i32 = wgsl.max 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():void -> %b1 {
  %b1 = block {
    %2:i32 = max 1i, 2i
    ret
  }
}
)";

    Run(Lower);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::wgsl::reader::lower
