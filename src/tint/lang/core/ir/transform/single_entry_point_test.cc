// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/core/ir/transform/single_entry_point.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::core::ir::transform {
namespace {

class IR_SingleEntryPointTest : public TransformTest {
  protected:
    /// @returns a new entry point called @p name that references @p refs
    Function* EntryPoint(const char* name, std::initializer_list<Value*> refs = {}) {
        auto* func = Func(name, std::move(refs));
        func->SetStage(Function::PipelineStage::kFragment);
        return func;
    }

    /// @returns a new function called @p name that references @p refs
    Function* Func(const char* name, std::initializer_list<Value*> refs = {}) {
        auto* func = b.Function(name, ty.void_());
        b.Append(func->Block(), [&] {
            for (auto* ref : refs) {
                if (auto* f = ref->As<Function>()) {
                    b.Call(f);
                } else {
                    b.Let(ref->Type())->SetValue(ref);
                }
            }
            b.Return(func);
        });
        return func;
    }

    /// @returns a new module-scope variable called @p name
    InstructionResult* Var(const char* name) {
        auto* var = b.Var<private_, i32>(name);
        mod.root_block->Append(var);
        return var->Result(0);
    }
};
using IR_SingleEntryPointDeathTest = IR_SingleEntryPointTest;

TEST_F(IR_SingleEntryPointTest, EntryPointNotFound) {
    EntryPoint("main");

    auto* src = R"(
%main = @fragment func():void {
  $B1: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    EXPECT_DEATH_IF_SUPPORTED({ Run(SingleEntryPoint, "foo"); }, "internal compiler error");
}

TEST_F(IR_SingleEntryPointTest, MultipleEntryPointsMatch) {
    EntryPoint("main");
    EntryPoint("main");

    auto* src = R"(
%main = @fragment func():void {
  $B1: {
    ret
  }
}
%main_1 = @fragment func():void {  # %main_1: 'main'
  $B2: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    EXPECT_DEATH_IF_SUPPORTED({ Run(SingleEntryPoint, "main"); }, "internal compiler error");
}

TEST_F(IR_SingleEntryPointTest, NoChangesNeeded) {
    EntryPoint("main");

    auto* src = R"(
%main = @fragment func():void {
  $B1: {
    ret
  }
}
)";

    auto* expect = src;

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "main");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, TwoEntryPoints) {
    EntryPoint("foo");
    EntryPoint("bar");

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    ret
  }
}
%bar = @fragment func():void {
  $B2: {
    ret
  }
}
)";

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, DirectFunctionCalls) {
    auto* f1 = Func("f1");
    auto* f2 = Func("f2");
    auto* f3 = Func("f3");

    EntryPoint("foo", {f1, f2});
    EntryPoint("bar", {f3});

    auto* src = R"(
%f1 = func():void {
  $B1: {
    ret
  }
}
%f2 = func():void {
  $B2: {
    ret
  }
}
%f3 = func():void {
  $B3: {
    ret
  }
}
%foo = @fragment func():void {
  $B4: {
    %5:void = call %f1
    %6:void = call %f2
    ret
  }
}
%bar = @fragment func():void {
  $B5: {
    %8:void = call %f3
    ret
  }
}
)";

    auto* expect = R"(
%f1 = func():void {
  $B1: {
    ret
  }
}
%f2 = func():void {
  $B2: {
    ret
  }
}
%foo = @fragment func():void {
  $B3: {
    %4:void = call %f1
    %5:void = call %f2
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, DirectVariables) {
    auto* v1 = Var("v1");
    auto* v2 = Var("v2");
    auto* v3 = Var("v3");

    EntryPoint("foo", {v1, v2});
    EntryPoint("bar", {v3});

    auto* src = R"(
$B1: {  # root
  %v1:ptr<private, i32, read_write> = var
  %v2:ptr<private, i32, read_write> = var
  %v3:ptr<private, i32, read_write> = var
}

%foo = @fragment func():void {
  $B2: {
    %5:ptr<private, i32, read_write> = let %v1
    %6:ptr<private, i32, read_write> = let %v2
    ret
  }
}
%bar = @fragment func():void {
  $B3: {
    %8:ptr<private, i32, read_write> = let %v3
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %v1:ptr<private, i32, read_write> = var
  %v2:ptr<private, i32, read_write> = var
}

%foo = @fragment func():void {
  $B2: {
    %4:ptr<private, i32, read_write> = let %v1
    %5:ptr<private, i32, read_write> = let %v2
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, TransitiveReferences) {
    Var("unused_var");
    Func("unused_func");

    auto* v1 = Var("v1");
    auto* v2 = Var("v2");
    auto* v3 = Var("v3");

    auto* f1 = Func("f1", {v2, v3});
    auto* f2 = Func("f2", {f1});
    auto* f3 = Func("f3", {v1, f2});

    EntryPoint("foo", {f3});

    auto* src = R"(
$B1: {  # root
  %unused_var:ptr<private, i32, read_write> = var
  %v1:ptr<private, i32, read_write> = var
  %v2:ptr<private, i32, read_write> = var
  %v3:ptr<private, i32, read_write> = var
}

%unused_func = func():void {
  $B2: {
    ret
  }
}
%f1 = func():void {
  $B3: {
    %7:ptr<private, i32, read_write> = let %v2
    %8:ptr<private, i32, read_write> = let %v3
    ret
  }
}
%f2 = func():void {
  $B4: {
    %10:void = call %f1
    ret
  }
}
%f3 = func():void {
  $B5: {
    %12:ptr<private, i32, read_write> = let %v1
    %13:void = call %f2
    ret
  }
}
%foo = @fragment func():void {
  $B6: {
    %15:void = call %f3
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %v1:ptr<private, i32, read_write> = var
  %v2:ptr<private, i32, read_write> = var
  %v3:ptr<private, i32, read_write> = var
}

%f1 = func():void {
  $B2: {
    %5:ptr<private, i32, read_write> = let %v2
    %6:ptr<private, i32, read_write> = let %v3
    ret
  }
}
%f2 = func():void {
  $B3: {
    %8:void = call %f1
    ret
  }
}
%f3 = func():void {
  $B4: {
    %10:ptr<private, i32, read_write> = let %v1
    %11:void = call %f2
    ret
  }
}
%foo = @fragment func():void {
  $B5: {
    %13:void = call %f3
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, RemoveMultipleFunctions) {
    auto* f1 = Func("f1");
    auto* f2 = Func("f2");
    auto* f3 = Func("f3");
    auto* f4 = Func("f4");
    auto* f5 = Func("f5");
    auto* f6 = Func("f6");
    auto* f7 = Func("f7");

    EntryPoint("foo", {f1, f5});
    EntryPoint("bar", {f2, f3, f4, f6, f7});

    auto* src = R"(
%f1 = func():void {
  $B1: {
    ret
  }
}
%f2 = func():void {
  $B2: {
    ret
  }
}
%f3 = func():void {
  $B3: {
    ret
  }
}
%f4 = func():void {
  $B4: {
    ret
  }
}
%f5 = func():void {
  $B5: {
    ret
  }
}
%f6 = func():void {
  $B6: {
    ret
  }
}
%f7 = func():void {
  $B7: {
    ret
  }
}
%foo = @fragment func():void {
  $B8: {
    %9:void = call %f1
    %10:void = call %f5
    ret
  }
}
%bar = @fragment func():void {
  $B9: {
    %12:void = call %f2
    %13:void = call %f3
    %14:void = call %f4
    %15:void = call %f6
    %16:void = call %f7
    ret
  }
}
)";

    auto* expect = R"(
%f1 = func():void {
  $B1: {
    ret
  }
}
%f5 = func():void {
  $B2: {
    ret
  }
}
%foo = @fragment func():void {
  $B3: {
    %4:void = call %f1
    %5:void = call %f5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

TEST_F(IR_SingleEntryPointTest, RemoveMultipleVariables) {
    auto* v1 = Var("v1");
    auto* v2 = Var("v2");
    auto* v3 = Var("v3");
    auto* v4 = Var("v4");
    auto* v5 = Var("v5");
    auto* v6 = Var("v6");
    auto* v7 = Var("v7");

    EntryPoint("foo", {v1, v5});
    EntryPoint("bar", {v1, v2, v3, v4, v5, v6, v7});

    auto* src = R"(
$B1: {  # root
  %v1:ptr<private, i32, read_write> = var
  %v2:ptr<private, i32, read_write> = var
  %v3:ptr<private, i32, read_write> = var
  %v4:ptr<private, i32, read_write> = var
  %v5:ptr<private, i32, read_write> = var
  %v6:ptr<private, i32, read_write> = var
  %v7:ptr<private, i32, read_write> = var
}

%foo = @fragment func():void {
  $B2: {
    %9:ptr<private, i32, read_write> = let %v1
    %10:ptr<private, i32, read_write> = let %v5
    ret
  }
}
%bar = @fragment func():void {
  $B3: {
    %12:ptr<private, i32, read_write> = let %v1
    %13:ptr<private, i32, read_write> = let %v2
    %14:ptr<private, i32, read_write> = let %v3
    %15:ptr<private, i32, read_write> = let %v4
    %16:ptr<private, i32, read_write> = let %v5
    %17:ptr<private, i32, read_write> = let %v6
    %18:ptr<private, i32, read_write> = let %v7
    ret
  }
}
)";

    auto* expect = R"(
$B1: {  # root
  %v1:ptr<private, i32, read_write> = var
  %v5:ptr<private, i32, read_write> = var
}

%foo = @fragment func():void {
  $B2: {
    %4:ptr<private, i32, read_write> = let %v1
    %5:ptr<private, i32, read_write> = let %v5
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    Run(SingleEntryPoint, "foo");

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
