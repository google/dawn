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

#include "src/tint/lang/msl/writer/helper_test.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::msl::writer {
namespace {

TEST_F(MslWriterTest, Loop) {
    auto* func = b.Function("a", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Body(), [&] { b.ExitLoop(l); });
        b.Append(l->Continuing(), [&] { b.NextIteration(l); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

#define TINT_ISOLATE_UB(VOLATILE_NAME) \
  {volatile bool VOLATILE_NAME = false; if (VOLATILE_NAME) break;}

void a() {
  {
    while(true) {
      TINT_ISOLATE_UB(tint_volatile_false)
      break;
    }
  }
}
)");
}

TEST_F(MslWriterTest, LoopContinueAndBreakIf) {
    auto* func = b.Function("a", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Body(), [&] { b.Continue(l); });
        b.Append(l->Continuing(), [&] { b.BreakIf(l, true); });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

#define TINT_ISOLATE_UB(VOLATILE_NAME) \
  {volatile bool VOLATILE_NAME = false; if (VOLATILE_NAME) break;}

void a() {
  {
    while(true) {
      TINT_ISOLATE_UB(tint_volatile_false)
      {
        if (true) { break; }
      }
      continue;
    }
  }
}
)");
}

TEST_F(MslWriterTest, LoopBodyVarInContinue) {
    auto* func = b.Function("a", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Body(), [&] {
            auto* v = b.Var("v", true);
            b.Continue(l);

            b.Append(l->Continuing(), [&] { b.BreakIf(l, v); });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

#define TINT_ISOLATE_UB(VOLATILE_NAME) \
  {volatile bool VOLATILE_NAME = false; if (VOLATILE_NAME) break;}

void a() {
  {
    while(true) {
      TINT_ISOLATE_UB(tint_volatile_false)
      bool v = true;
      {
        if (v) { break; }
      }
      continue;
    }
  }
}
)");
}

TEST_F(MslWriterTest, LoopInitializer) {
    auto* func = b.Function("a", ty.void_());
    b.Append(func->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Initializer(), [&] {
            auto* v = b.Var("v", true);
            b.NextIteration(l);

            b.Append(l->Body(), [&] { b.Continue(l); });
            b.Append(l->Continuing(), [&] { b.BreakIf(l, v); });
        });
        b.Return(func);
    });

    ASSERT_TRUE(Generate()) << err_ << output_.msl;
    EXPECT_EQ(output_.msl, R"(#include <metal_stdlib>
using namespace metal;

#define TINT_ISOLATE_UB(VOLATILE_NAME) \
  {volatile bool VOLATILE_NAME = false; if (VOLATILE_NAME) break;}

void a() {
  {
    bool v = true;
    while(true) {
      TINT_ISOLATE_UB(tint_volatile_false)
      {
        if (v) { break; }
      }
      continue;
    }
  }
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
