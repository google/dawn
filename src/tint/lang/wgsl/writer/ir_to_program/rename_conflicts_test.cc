// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/writer/ir_to_program/rename_conflicts.h"

#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/matrix.h"

namespace tint::wgsl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IRToProgramRenameConflictsTest : public testing::Test {
  public:
    /// Transforms the module, using the transforms `TRANSFORMS`.
    void Run() {
        // Validate the input IR.
        {
            auto res = core::ir::Validate(mod);
            EXPECT_TRUE(res) << res.Failure().reason.str();
            if (!res) {
                return;
            }
        }

        // Run the transforms.
        auto result = RenameConflicts(&mod);
        EXPECT_TRUE(result) << result.Failure();

        // Validate the output IR.
        auto res = core::ir::Validate(mod);
        EXPECT_TRUE(res) << res.Failure().reason.str();
    }

    /// @returns the transformed module as a disassembled string
    std::string str() {
        core::ir::Disassembler dis(mod);
        return "\n" + dis.Disassemble();
    }

  protected:
    /// The test IR module.
    core::ir::Module mod;
    /// The test IR builder.
    core::ir::Builder b{mod};
    /// The type manager.
    core::type::Manager& ty{mod.Types()};
};

TEST_F(IRToProgramRenameConflictsTest, NoModify_SingleNamedRootBlockVar) {
    b.Append(mod.root_block, [&] { b.ir.SetName(b.Var(ty.ptr<private_, i32>()), "v"); });

    auto* src = R"(
%b1 = block {  # root
  %v:ptr<private, i32, read_write> = var
}

)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_TwoRootBlockVarsWithSameName) {
    b.Append(mod.root_block, [&] {
        b.ir.SetName(b.Var(ty.ptr<private_, i32>()), "v");
        b.ir.SetName(b.Var(ty.ptr<private_, u32>()), "v");
    });

    auto* src = R"(
%b1 = block {  # root
  %v:ptr<private, i32, read_write> = var
  %v_1:ptr<private, u32, read_write> = var  # %v_1: 'v'
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %v:ptr<private, i32, read_write> = var
  %v_1:ptr<private, u32, read_write> = var
}

)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_RootBlockVarAndStructWithSameName) {
    auto* s = ty.Struct(b.ir.symbols.New("v"), {{b.ir.symbols.New("x"), ty.i32()}});
    b.Append(mod.root_block, [&] { b.ir.SetName(b.Var(ty.ptr(function, s)), "v"); });

    auto* src = R"(
v = struct @align(4) {
  x:i32 @offset(0)
}

%b1 = block {  # root
  %v:ptr<function, v, read_write> = var
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
v = struct @align(4) {
  x:i32 @offset(0)
}

%b1 = block {  # root
  %v_1:ptr<function, v, read_write> = var
}

)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_RootBlockVarAndFnWithSameName) {
    b.Append(mod.root_block, [&] { b.ir.SetName(b.Var(ty.ptr<private_, i32>()), "v"); });

    auto* fn = b.Function("v", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto* src = R"(
%b1 = block {  # root
  %v:ptr<private, i32, read_write> = var
}

%v_1 = func():void -> %b2 {  # %v_1: 'v'
  %b2 = block {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %v:ptr<private, i32, read_write> = var
}

%v_1 = func():void -> %b2 {
  %b2 = block {
    ret
  }
}
)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, NoModify_RootBlockVar_ShadowedBy_FnVar) {
    b.Append(mod.root_block, [&] {
        auto* outer = b.Var(ty.ptr<private_, i32>());
        b.ir.SetName(outer, "v");

        auto* fn = b.Function("f", ty.i32());
        b.Append(fn->Block(), [&] {
            auto* load_outer = b.Load(outer);

            auto* inner = b.Var(ty.ptr<function, f32>());
            b.ir.SetName(inner, "v");

            auto* load_inner = b.Load(inner);
            b.Return(fn, b.Add(ty.i32(), load_outer, load_inner));
        });
    });

    auto* src = R"(
%b1 = block {  # root
  %v:ptr<private, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %3:i32 = load %v
    %v_1:ptr<function, f32, read_write> = var  # %v_1: 'v'
    %5:f32 = load %v_1
    %6:i32 = add %3, %5
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_RootBlockVar_ShadowedBy_FnVar) {
    b.Append(mod.root_block, [&] {
        auto* outer = b.Var(ty.ptr<private_, i32>());
        b.ir.SetName(outer, "v");

        auto* fn = b.Function("f", ty.i32());
        b.Append(fn->Block(), [&] {
            auto* inner = b.Var(ty.ptr<function, f32>());
            b.ir.SetName(inner, "v");

            auto* load_outer = b.Load(outer);
            auto* load_inner = b.Load(inner);
            b.Return(fn, b.Add(ty.i32(), load_outer, load_inner));
        });
    });

    auto* src = R"(
%b1 = block {  # root
  %v:ptr<private, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %v_1:ptr<function, f32, read_write> = var  # %v_1: 'v'
    %4:i32 = load %v
    %5:f32 = load %v_1
    %6:i32 = add %4, %5
    ret %6
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %v:ptr<private, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %v_1:ptr<function, f32, read_write> = var
    %4:i32 = load %v
    %5:f32 = load %v_1
    %6:i32 = add %4, %5
    ret %6
  }
}
)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, NoModify_FnVar_ShadowedBy_IfVar) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* outer = b.Var(ty.ptr<function, f32>());
        b.ir.SetName(outer, "v");

        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            auto* load_outer = b.Load(outer);

            auto* inner = b.Var(ty.ptr<function, f32>());
            b.ir.SetName(inner, "v");

            auto* load_inner = b.Load(inner);
            b.Return(fn, b.Add(ty.i32(), load_outer, load_inner));
        });

        b.Unreachable();
    });

    auto* src = R"(
%f = func():i32 -> %b1 {
  %b1 = block {
    %v:ptr<function, f32, read_write> = var
    if true [t: %b2] {  # if_1
      %b2 = block {  # true
        %3:f32 = load %v
        %v_1:ptr<function, f32, read_write> = var  # %v_1: 'v'
        %5:f32 = load %v_1
        %6:i32 = add %3, %5
        ret %6
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_FnVar_ShadowedBy_IfVar) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* outer = b.Var(ty.ptr<function, f32>());
        b.ir.SetName(outer, "v");

        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            auto* inner = b.Var(ty.ptr<function, f32>());
            b.ir.SetName(inner, "v");

            auto* load_outer = b.Load(outer);
            auto* load_inner = b.Load(inner);
            b.Return(fn, b.Add(ty.i32(), load_outer, load_inner));
        });

        b.Unreachable();
    });

    auto* src = R"(
%f = func():i32 -> %b1 {
  %b1 = block {
    %v:ptr<function, f32, read_write> = var
    if true [t: %b2] {  # if_1
      %b2 = block {  # true
        %v_1:ptr<function, f32, read_write> = var  # %v_1: 'v'
        %4:f32 = load %v
        %5:f32 = load %v_1
        %6:i32 = add %4, %5
        ret %6
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 -> %b1 {
  %b1 = block {
    %v:ptr<function, f32, read_write> = var
    if true [t: %b2] {  # if_1
      %b2 = block {  # true
        %v_1:ptr<function, f32, read_write> = var
        %4:f32 = load %v
        %5:f32 = load %v_1
        %6:i32 = add %4, %5
        ret %6
      }
    }
    unreachable
  }
}
)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, NoModify_LoopInitVar_ShadowedBy_LoopBodyVar) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            auto* outer = b.Var(ty.ptr<function, f32>());
            b.ir.SetName(outer, "v");
            b.NextIteration(loop);

            b.Append(loop->Body(), [&] {
                auto* load_outer = b.Load(outer);

                auto* inner = b.Var(ty.ptr<function, f32>());
                b.ir.SetName(inner, "v");

                auto* load_inner = b.Load(inner);
                b.Return(fn, b.Add(ty.i32(), load_outer, load_inner));
            });
        });

        b.Unreachable();
    });

    auto* src = R"(
%f = func():i32 -> %b1 {
  %b1 = block {
    loop [i: %b2, b: %b3] {  # loop_1
      %b2 = block {  # initializer
        %v:ptr<function, f32, read_write> = var
        next_iteration %b3
      }
      %b3 = block {  # body
        %3:f32 = load %v
        %v_1:ptr<function, f32, read_write> = var  # %v_1: 'v'
        %5:f32 = load %v_1
        %6:i32 = add %3, %5
        ret %6
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_LoopInitVar_ShadowedBy_LoopBodyVar) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] {
            auto* outer = b.Var(ty.ptr<function, f32>());
            b.ir.SetName(outer, "v");
            b.NextIteration(loop);

            b.Append(loop->Body(), [&] {
                auto* inner = b.Var(ty.ptr<function, f32>());
                b.ir.SetName(inner, "v");

                auto* load_outer = b.Load(outer);
                auto* load_inner = b.Load(inner);
                b.Return(fn, b.Add(ty.i32(), load_outer, load_inner));
            });
        });

        b.Unreachable();
    });

    auto* src = R"(
%f = func():i32 -> %b1 {
  %b1 = block {
    loop [i: %b2, b: %b3] {  # loop_1
      %b2 = block {  # initializer
        %v:ptr<function, f32, read_write> = var
        next_iteration %b3
      }
      %b3 = block {  # body
        %v_1:ptr<function, f32, read_write> = var  # %v_1: 'v'
        %4:f32 = load %v
        %5:f32 = load %v_1
        %6:i32 = add %4, %5
        ret %6
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 -> %b1 {
  %b1 = block {
    loop [i: %b2, b: %b3] {  # loop_1
      %b2 = block {  # initializer
        %v:ptr<function, f32, read_write> = var
        next_iteration %b3
      }
      %b3 = block {  # body
        %v_1:ptr<function, f32, read_write> = var
        %4:f32 = load %v
        %5:f32 = load %v_1
        %6:i32 = add %4, %5
        ret %6
      }
    }
    unreachable
  }
}
)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, NoModify_LoopBodyVar_ShadowedBy_LoopContVar) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop); });
        b.Append(loop->Body(), [&] {
            auto* outer = b.Var(ty.ptr<function, f32>());
            b.ir.SetName(outer, "v");
            b.Continue(loop);

            b.Append(loop->Continuing(), [&] {
                auto* load_outer = b.Load(outer);

                auto* inner = b.Var(ty.ptr<function, f32>());
                b.ir.SetName(inner, "v");

                auto* load_inner = b.Load(inner);
                b.Return(fn, b.Add(ty.i32(), load_outer, load_inner));
            });
        });

        b.Unreachable();
    });

    auto* src = R"(
%f = func():i32 -> %b1 {
  %b1 = block {
    loop [i: %b2, b: %b3, c: %b4] {  # loop_1
      %b2 = block {  # initializer
        next_iteration %b3
      }
      %b3 = block {  # body
        %v:ptr<function, f32, read_write> = var
        continue %b4
      }
      %b4 = block {  # continuing
        %3:f32 = load %v
        %v_1:ptr<function, f32, read_write> = var  # %v_1: 'v'
        %5:f32 = load %v_1
        %6:i32 = add %3, %5
        ret %6
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_LoopBodyVar_ShadowedBy_LoopContVar) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop); });
        b.Append(loop->Body(), [&] {
            auto* outer = b.Var(ty.ptr<function, f32>());
            b.ir.SetName(outer, "v");
            b.Continue(loop);

            b.Append(loop->Continuing(), [&] {
                auto* inner = b.Var(ty.ptr<function, f32>());
                b.ir.SetName(inner, "v");

                auto* load_outer = b.Load(outer);
                auto* load_inner = b.Load(inner);
                b.Return(fn, b.Add(ty.i32(), load_outer, load_inner));
            });
        });

        b.Unreachable();
    });

    auto* src = R"(
%f = func():i32 -> %b1 {
  %b1 = block {
    loop [i: %b2, b: %b3, c: %b4] {  # loop_1
      %b2 = block {  # initializer
        next_iteration %b3
      }
      %b3 = block {  # body
        %v:ptr<function, f32, read_write> = var
        continue %b4
      }
      %b4 = block {  # continuing
        %v_1:ptr<function, f32, read_write> = var  # %v_1: 'v'
        %4:f32 = load %v
        %5:f32 = load %v_1
        %6:i32 = add %4, %5
        ret %6
      }
    }
    unreachable
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 -> %b1 {
  %b1 = block {
    loop [i: %b2, b: %b3, c: %b4] {  # loop_1
      %b2 = block {  # initializer
        next_iteration %b3
      }
      %b3 = block {  # body
        %v:ptr<function, f32, read_write> = var
        continue %b4
      }
      %b4 = block {  # continuing
        %v_1:ptr<function, f32, read_write> = var
        %4:f32 = load %v
        %5:f32 = load %v_1
        %6:i32 = add %4, %5
        ret %6
      }
    }
    unreachable
  }
}
)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_BuiltinScalar_ShadowedBy_Param) {
    auto* fn = b.Function("f", ty.void_());
    auto* p = b.FunctionParam(ty.i32());
    b.ir.SetName(p, "i32");
    fn->SetParams({p});

    b.Append(fn->Block(), [&] {
        b.Var(ty.ptr<function, i32>());
        b.Return(fn);
    });

    auto* src = R"(
%f = func(%i32:i32):void -> %b1 {
  %b1 = block {
    %3:ptr<function, i32, read_write> = var
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func(%i32_1:i32):void -> %b1 {
  %b1 = block {
    %3:ptr<function, i32, read_write> = var
    ret
  }
}
)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, NoModify_BuiltinVector_ShadowedBy_Param) {
    auto* fn = b.Function("f", ty.void_());
    auto* p = b.FunctionParam(ty.i32());
    b.ir.SetName(p, "vec2");
    fn->SetParams({p});

    b.Append(fn->Block(), [&] {
        b.Var(ty.ptr<function, vec3<i32>>());
        b.Return(fn);
    });

    auto* src = R"(
%f = func(%vec2:i32):void -> %b1 {
  %b1 = block {
    %3:ptr<function, vec3<i32>, read_write> = var
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_BuiltinVector_ShadowedBy_Param) {
    auto* fn = b.Function("f", ty.void_());
    auto* p = b.FunctionParam(ty.i32());
    b.ir.SetName(p, "vec3");
    fn->SetParams({p});

    b.Append(fn->Block(), [&] {
        b.Var(ty.ptr<function, vec3<i32>>());
        b.Return(fn);
    });

    auto* src = R"(
%f = func(%vec3:i32):void -> %b1 {
  %b1 = block {
    %3:ptr<function, vec3<i32>, read_write> = var
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func(%vec3_1:i32):void -> %b1 {
  %b1 = block {
    %3:ptr<function, vec3<i32>, read_write> = var
    ret
  }
}
)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, NoModify_BuiltinMatrix_ShadowedBy_Param) {
    auto* fn = b.Function("f", ty.void_());
    auto* p = b.FunctionParam(ty.i32());
    b.ir.SetName(p, "mat3x2");
    fn->SetParams({p});

    b.Append(fn->Block(), [&] {
        b.Var(ty.ptr<function, mat2x4<f32>>());
        b.Return(fn);
    });

    auto* src = R"(
%f = func(%mat3x2:i32):void -> %b1 {
  %b1 = block {
    %3:ptr<function, mat2x4<f32>, read_write> = var
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_BuiltinMatrix_ShadowedBy_Param) {
    auto* fn = b.Function("f", ty.void_());
    auto* p = b.FunctionParam(ty.i32());
    b.ir.SetName(p, "mat2x4");
    fn->SetParams({p});

    b.Append(fn->Block(), [&] {
        b.Var(ty.ptr<function, mat2x4<f32>>());
        b.Return(fn);
    });

    auto* src = R"(
%f = func(%mat2x4:i32):void -> %b1 {
  %b1 = block {
    %3:ptr<function, mat2x4<f32>, read_write> = var
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func(%mat2x4_1:i32):void -> %b1 {
  %b1 = block {
    %3:ptr<function, mat2x4<f32>, read_write> = var
    ret
  }
}
)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, NoModify_BuiltinScalar_ShadowedBy_FnVar) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var(ty.ptr<function, i32>());
        b.ir.SetName(v, "f32");

        b.Return(fn, b.Construct(ty.i32()));
    });

    auto* src = R"(
%f = func():i32 -> %b1 {
  %b1 = block {
    %f32:ptr<function, i32, read_write> = var
    %3:i32 = construct
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_BuiltinScalar_ShadowedBy_FnVar) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* v = b.Var(ty.ptr<function, i32>());
        b.ir.SetName(v, "i32");

        b.Return(fn, b.Construct(ty.i32()));
    });

    auto* src = R"(
%f = func():i32 -> %b1 {
  %b1 = block {
    %i32:ptr<function, i32, read_write> = var
    %3:i32 = construct
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():i32 -> %b1 {
  %b1 = block {
    %i32_1:ptr<function, i32, read_write> = var
    %3:i32 = construct
    ret %3
  }
}
)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, NoModify_BuiltinScalar_ShadowedBy_NamedInst) {
    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {
        auto* i = b.Add(ty.i32(), 1_i, 2_i);
        b.ir.SetName(i, "i32");

        b.Return(fn, i);
    });

    auto* src = R"(
%f = func():i32 -> %b1 {
  %b1 = block {
    %i32:i32 = add 1i, 2i
    ret %i32
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_BuiltinScalar_ShadowedBy_NamedInst) {
    auto* fn = b.Function("f", ty.f32());
    b.Append(fn->Block(), [&] {
        auto* i = b.Add(ty.i32(), 1_i, 2_i);
        b.ir.SetName(i, "f32");

        b.Return(fn, b.Construct(ty.f32(), i));
    });

    auto* src = R"(
%f = func():f32 -> %b1 {
  %b1 = block {
    %f32:i32 = add 1i, 2i
    %3:f32 = construct %f32
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():f32 -> %b1 {
  %b1 = block {
    %f32_1:i32 = add 1i, 2i
    %3:f32 = construct %f32_1
    ret %3
  }
}
)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, NoModify_BuiltinAddressSpace_ShadowedBy_RootBlockVar) {
    b.Append(mod.root_block, [&] {  //
        b.ir.SetName(b.Var(ty.ptr<private_, i32>()), "function");
    });

    auto* src = R"(
%b1 = block {  # root
  %function:ptr<private, i32, read_write> = var
}

)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_BuiltinAddressSpace_ShadowedBy_RootBlockVar) {
    b.Append(mod.root_block, [&] {  //
        b.ir.SetName(b.Var(ty.ptr<private_, i32>()), "private");
    });

    auto* src = R"(
%b1 = block {  # root
  %private:ptr<private, i32, read_write> = var
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %private_1:ptr<private, i32, read_write> = var
}

)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, NoModify_BuiltinAccess_ShadowedBy_RootBlockVar) {
    b.Append(mod.root_block, [&] {  //
        b.ir.SetName(b.Var(ty.ptr<private_, i32>()), "read");
    });

    auto* src = R"(
%b1 = block {  # root
  %read:ptr<private, i32, read_write> = var
}

)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_BuiltinAccess_ShadowedBy_RootBlockVar) {
    b.Append(mod.root_block, [&] {  //
        b.ir.SetName(b.Var(ty.ptr<private_, i32>()), "read_write");
    });

    auto* src = R"(
%b1 = block {  # root
  %read_write:ptr<private, i32, read_write> = var
}

)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %read_write_1:ptr<private, i32, read_write> = var
}

)";

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, NoModify_BuiltinFn_ShadowedBy_RootBlockVar) {
    b.Append(mod.root_block, [&] {  //
        auto* v = b.Var(ty.ptr<private_, i32>());
        b.ir.SetName(v, "min");
    });

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {  //
        auto* res = b.Call(ty.i32(), core::BuiltinFn::kMax, 1_i, 2_i)->Result();
        b.Return(fn, res);
    });

    auto* src = R"(
%b1 = block {  # root
  %min:ptr<private, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %3:i32 = max 1i, 2i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run();

    EXPECT_EQ(expect, str());
}

TEST_F(IRToProgramRenameConflictsTest, Conflict_BuiltinFn_ShadowedBy_RootBlockVar) {
    b.Append(mod.root_block, [&] {  //
        auto* v = b.Var(ty.ptr<private_, i32>());
        b.ir.SetName(v, "max");
    });

    auto* fn = b.Function("f", ty.i32());
    b.Append(fn->Block(), [&] {  //
        auto* res = b.Call(ty.i32(), core::BuiltinFn::kMax, 1_i, 2_i)->Result();
        b.Return(fn, res);
    });

    auto* src = R"(
%b1 = block {  # root
  %max:ptr<private, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %3:i32 = max 1i, 2i
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %max_1:ptr<private, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %3:i32 = max 1i, 2i
    ret %3
  }
}
)";

    Run();

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::wgsl::writer
