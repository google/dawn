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

#include "src/tint/lang/hlsl/writer/raise/fxc_polyfill.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/number.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

using HlslWriterFxcPolyfillTest = core::ir::transform::TransformTest;

// No change, no switch
TEST_F(HlslWriterFxcPolyfillTest, NoSwitch) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] { b.Return(func); });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(FxcPolyfill);

    EXPECT_EQ(expect, str());
}

// No change, switch with case and default
TEST_F(HlslWriterFxcPolyfillTest, SwitchCaseAndDefault) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(1_i);
        b.Append(b.Case(s, {b.Constant(0_i)}), [&] { b.ExitSwitch(s); });
        b.Append(b.DefaultCase(s), [&] { b.ExitSwitch(s); });

        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    switch 1i [c: (0i, $B2), c: (default, $B3)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
      $B3: {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(FxcPolyfill);

    EXPECT_EQ(expect, str());
}

// No change, switch with multi-selector default case
TEST_F(HlslWriterFxcPolyfillTest, SwitchMultiSelectorDefault) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(1_i);
        b.Append(b.Case(s, {b.Constant(0_i), nullptr}), [&] { b.ExitSwitch(s); });
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    switch 1i [c: (0i default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;
    Run(FxcPolyfill);

    EXPECT_EQ(expect, str());
}

// Switch body just has a ExitSwitch
TEST_F(HlslWriterFxcPolyfillTest, Switch) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);
    b.Append(func->Block(), [&] {
        auto* s = b.Switch(1_i);
        b.Append(b.DefaultCase(s), [&] { b.ExitSwitch(s); });
        b.Return(func);
    });

    auto* src = R"(
%foo = @fragment func():void {
  $B1: {
    switch 1i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
%foo = @fragment func():void {
  $B1: {
    switch 1i [c: (default 0i, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";

    Run(FxcPolyfill);

    EXPECT_EQ(expect, str());
}

// Switch body with assignment
TEST_F(HlslWriterFxcPolyfillTest, SwitchWithAssignment) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);

    auto* a = b.Var<private_>("a", b.Zero<i32>());
    b.ir.root_block->Append(a);

    b.Append(func->Block(), [&] {
        auto* s = b.Switch(1_i);
        b.Append(b.DefaultCase(s), [&] {
            b.Store(a, 1_i);
            b.ExitSwitch(s);
        });
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<private, i32, read_write> = var, 0i
}

%foo = @fragment func():void {
  $B2: {
    switch 1i [c: (default, $B3)] {  # switch_1
      $B3: {  # case
        store %a, 1i
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %a:ptr<private, i32, read_write> = var, 0i
}

%foo = @fragment func():void {
  $B2: {
    switch 1i [c: (default 0i, $B3)] {  # switch_1
      $B3: {  # case
        store %a, 1i
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";

    Run(FxcPolyfill);

    EXPECT_EQ(expect, str());
}

// Switch with if with break
TEST_F(HlslWriterFxcPolyfillTest, SwitchWithIfBreak) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);

    auto* a = b.Var<private_>("a", b.Zero<i32>());
    b.ir.root_block->Append(a);

    b.Append(func->Block(), [&] {
        auto* s = b.Switch(1_i);
        b.Append(b.DefaultCase(s), [&] {
            auto* i = b.If(true);
            b.Append(i->True(), [&] {
                b.Store(a, 1_i);
                b.ExitSwitch(s);
            });
            b.ExitSwitch(s);
        });
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<private, i32, read_write> = var, 0i
}

%foo = @fragment func():void {
  $B2: {
    switch 1i [c: (default, $B3)] {  # switch_1
      $B3: {  # case
        if true [t: $B4] {  # if_1
          $B4: {  # true
            store %a, 1i
            exit_switch  # switch_1
          }
        }
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %a:ptr<private, i32, read_write> = var, 0i
}

%foo = @fragment func():void {
  $B2: {
    switch 1i [c: (default 0i, $B3)] {  # switch_1
      $B3: {  # case
        if true [t: $B4] {  # if_1
          $B4: {  # true
            store %a, 1i
            exit_switch  # switch_1
          }
        }
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";

    Run(FxcPolyfill);

    EXPECT_EQ(expect, str());
}

// Switch with loop with break
TEST_F(HlslWriterFxcPolyfillTest, SwitchWithLoopBreak) {
    auto* func = b.Function("foo", ty.void_(), core::ir::Function::PipelineStage::kFragment);

    auto* a = b.Var<private_>("a", b.Zero<i32>());
    b.ir.root_block->Append(a);

    b.Append(func->Block(), [&] {
        auto* s = b.Switch(1_i);
        b.Append(b.DefaultCase(s), [&] {
            auto* l = b.Loop();
            b.Append(l->Body(), [&] {
                b.Store(a, 1_i);
                b.ExitLoop(l);
            });
            b.ExitSwitch(s);
        });
        b.Return(func);
    });

    auto* src = R"(
$B1: {  # root
  %a:ptr<private, i32, read_write> = var, 0i
}

%foo = @fragment func():void {
  $B2: {
    switch 1i [c: (default, $B3)] {  # switch_1
      $B3: {  # case
        loop [b: $B4] {  # loop_1
          $B4: {  # body
            store %a, 1i
            exit_loop  # loop_1
          }
        }
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";

    EXPECT_EQ(src, str());

    auto* expect = R"(
$B1: {  # root
  %a:ptr<private, i32, read_write> = var, 0i
}

%foo = @fragment func():void {
  $B2: {
    switch 1i [c: (default 0i, $B3)] {  # switch_1
      $B3: {  # case
        loop [b: $B4] {  # loop_1
          $B4: {  # body
            store %a, 1i
            exit_loop  # loop_1
          }
        }
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)";

    Run(FxcPolyfill);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::hlsl::writer::raise
