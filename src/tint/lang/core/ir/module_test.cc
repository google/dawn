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

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/var.h"

namespace tint::ir {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using IR_ModuleTest = IRTestHelper;

TEST_F(IR_ModuleTest, NameOfUnnamed) {
    auto* v = b.Var(ty.ptr<function, i32>());
    EXPECT_FALSE(mod.NameOf(v).IsValid());
}

TEST_F(IR_ModuleTest, SetName) {
    auto* v = b.Var(ty.ptr<function, i32>());
    mod.SetName(v, "a");
    EXPECT_EQ(mod.NameOf(v).Name(), "a");
}

TEST_F(IR_ModuleTest, SetNameRename) {
    auto* v = b.Var(ty.ptr<function, i32>());
    mod.SetName(v, "a");
    mod.SetName(v, "b");
    EXPECT_EQ(mod.NameOf(v).Name(), "b");
}

}  // namespace
}  // namespace tint::ir
