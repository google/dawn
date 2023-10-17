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

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/wgsl/writer/raise/raise.h"

namespace tint::wgsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using WgslWriter_RaiseTest = core::ir::transform::TransformTest;

TEST_F(WgslWriter_RaiseTest, BuiltinConversion) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Call(ty.i32(), core::BuiltinFn::kMax, i32(1), i32(2));
        b.Return(f);
    });

    auto* src = R"(
%f = func():void -> %b1 {
  %b1 = block {
    %2:i32 = max 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():void -> %b1 {
  %b1 = block {
    %2:i32 = wgsl.max 1i, 2i
    ret
  }
}
)";

    Run(Raise);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_RaiseTest, WorkgroupBarrier) {
    auto* W = b.Var<workgroup, i32, read_write>("W");
    b.ir.root_block->Append(W);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        auto* load = b.Load(W);
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(f, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %W:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %3:void = workgroupBarrier
    %4:i32 = load %W
    %5:void = workgroupBarrier
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %W:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %3:i32 = wgsl.workgroupUniformLoad %W
    ret %3
  }
}
)";

    Run(Raise);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_RaiseTest, WorkgroupBarrier_NoMatch) {
    auto* W = b.Var<workgroup, i32, read_write>("W");
    b.ir.root_block->Append(W);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Store(W, 42_i);  // Prevents pattern match
        auto* load = b.Load(W);
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(f, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %W:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %3:void = workgroupBarrier
    store %W, 42i
    %4:i32 = load %W
    %5:void = workgroupBarrier
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %W:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %3:void = wgsl.workgroupBarrier
    store %W, 42i
    %4:i32 = load %W
    %5:void = wgsl.workgroupBarrier
    ret %4
  }
}
)";

    Run(Raise);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::wgsl::writer::raise
