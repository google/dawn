// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/raise/polyfill_nested_aggregate_construct.h"

#include "src/tint/lang/core/ir/transform/helper_test.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_PolyfillNestedAggregateConstructTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_PolyfillNestedAggregateConstructTest, NoModify_FlatStruct) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.Register("a"), ty.u32()},
                                            {mod.symbols.Register("v"), ty.vec2f()},
                                            {mod.symbols.Register("m"), ty.mat2x2<f32>()},
                                        });
    auto* a = b.FunctionParam("a", ty.u32());
    auto* v = b.FunctionParam("v", ty.vec2f());
    auto* m = b.FunctionParam("m", ty.mat2x2<f32>());
    auto* func = b.Function("foo", str_ty);
    func->SetParams({a, v, m});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(str_ty, a, v, m);
        b.Return(func, result);
    });

    auto* src = R"(
S = struct @align(8) {
  a:u32 @offset(0)
  v:vec2<f32> @offset(8)
  m:mat2x2<f32> @offset(16)
}

%foo = func(%a:u32, %v:vec2<f32>, %m:mat2x2<f32>):S {
  $B1: {
    %5:S = construct %a, %v, %m
    ret %5
  }
}
)";
    EXPECT_EQ(src, str());

    Run(PolyfillNestedAggregateConstruct);

    EXPECT_EQ(src, str());
}

TEST_F(SpirvWriter_PolyfillNestedAggregateConstructTest, NoModify_FlatArray) {
    auto* a = b.FunctionParam("a", ty.u32());
    auto* func = b.Function("foo", ty.array<u32, 2>());
    func->SetParams({a});

    b.Append(func->Block(), [&] {
        auto* result = b.Construct(ty.array<u32, 2>(), a, a);
        b.Return(func, result);
    });

    auto* src = R"(
%foo = func(%a:u32):array<u32, 2> {
  $B1: {
    %3:array<u32, 2> = construct %a, %a
    ret %3
  }
}
)";
    EXPECT_EQ(src, str());

    Run(PolyfillNestedAggregateConstruct);

    EXPECT_EQ(src, str());
}

TEST_F(SpirvWriter_PolyfillNestedAggregateConstructTest, NoModify_ZeroValueNestedStruct) {
    auto* inner = ty.Struct(mod.symbols.New("S"), {
                                                      {mod.symbols.Register("a"), ty.u32()},
                                                      {mod.symbols.Register("b"), ty.u32()},
                                                  });
    auto* outer = ty.Struct(mod.symbols.New("T"), {
                                                      {mod.symbols.Register("x"), inner},
                                                      {mod.symbols.Register("y"), inner},
                                                  });
    auto* func = b.Function("foo", outer);

    b.Append(func->Block(), [&] {
        auto* result = mod.CreateInstruction<core::ir::Construct>(b.InstructionResult(outer));
        func->Block()->Append(result);
        b.Return(func, result);
    });

    auto* src = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

T = struct @align(4) {
  x:S @offset(0)
  y:S @offset(8)
}

%foo = func():T {
  $B1: {
    %2:T = construct
    ret %2
  }
}
)";
    EXPECT_EQ(src, str());

    Run(PolyfillNestedAggregateConstruct);

    EXPECT_EQ(src, str());
}

TEST_F(SpirvWriter_PolyfillNestedAggregateConstructTest, StructOfStruct) {
    auto* inner = ty.Struct(mod.symbols.New("S"), {
                                                      {mod.symbols.Register("a"), ty.u32()},
                                                      {mod.symbols.Register("b"), ty.u32()},
                                                  });
    auto* outer = ty.Struct(mod.symbols.New("T"), {
                                                      {mod.symbols.Register("x"), inner},
                                                      {mod.symbols.Register("y"), inner},
                                                  });
    auto* v = b.FunctionParam("v", ty.u32());
    auto* func = b.Function("foo", outer);
    func->SetParams({v});

    b.Append(func->Block(), [&] {
        auto* t = b.Construct(inner, v, v);
        auto* u = b.Construct(outer, t, t);
        b.Return(func, u);
    });

    auto* src = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

T = struct @align(4) {
  x:S @offset(0)
  y:S @offset(8)
}

%foo = func(%v:u32):T {
  $B1: {
    %3:S = construct %v, %v
    %4:T = construct %3, %3
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

T = struct @align(4) {
  x:S @offset(0)
  y:S @offset(8)
}

%foo = func(%v:u32):T {
  $B1: {
    %3:S = construct %v, %v
    %4:ptr<function, T, read_write> = var
    %5:ptr<function, S, read_write> = access %4, 0u
    store %5, %3
    %6:ptr<function, S, read_write> = access %4, 1u
    store %6, %3
    %7:T = load %4
    ret %7
  }
}
)";

    Run(PolyfillNestedAggregateConstruct);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_PolyfillNestedAggregateConstructTest, StructOfMixedMembers) {
    auto* inner = ty.Struct(mod.symbols.New("S"), {
                                                      {mod.symbols.Register("a"), ty.u32()},
                                                      {mod.symbols.Register("b"), ty.u32()},
                                                  });
    auto* arr_ty = ty.array<u32, 2>();
    auto* outer = ty.Struct(mod.symbols.New("T"), {
                                                      {mod.symbols.Register("k"), ty.u32()},
                                                      {mod.symbols.Register("x"), inner},
                                                      {mod.symbols.Register("y"), arr_ty},
                                                  });
    auto* k = b.FunctionParam("k", ty.u32());
    auto* s = b.FunctionParam("s", inner);
    auto* a = b.FunctionParam("a", arr_ty);
    auto* func = b.Function("foo", outer);
    func->SetParams({k, s, a});

    b.Append(func->Block(), [&] {
        auto* u = b.Construct(outer, k, s, a);
        b.Return(func, u);
    });

    auto* expect = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

T = struct @align(4) {
  k:u32 @offset(0)
  x:S @offset(4)
  y:array<u32, 2> @offset(12)
}

%foo = func(%k:u32, %s:S, %a:array<u32, 2>):T {
  $B1: {
    %5:ptr<function, T, read_write> = var
    %6:ptr<function, u32, read_write> = access %5, 0u
    store %6, %k
    %7:ptr<function, S, read_write> = access %5, 1u
    store %7, %s
    %8:ptr<function, array<u32, 2>, read_write> = access %5, 2u
    store %8, %a
    %9:T = load %5
    ret %9
  }
}
)";

    Run(PolyfillNestedAggregateConstruct);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_PolyfillNestedAggregateConstructTest, ArrayOfArray) {
    auto* inner_ty = ty.array<u32, 2>();
    auto* outer_ty = ty.array(inner_ty, 2u);
    auto* v = b.FunctionParam("v", ty.u32());
    auto* func = b.Function("foo", outer_ty);
    func->SetParams({v});

    b.Append(func->Block(), [&] {
        auto* t = b.Construct(inner_ty, v, v);
        auto* u = b.Construct(outer_ty, t, t);
        b.Return(func, u);
    });

    auto* expect = R"(
%foo = func(%v:u32):array<array<u32, 2>, 2> {
  $B1: {
    %3:array<u32, 2> = construct %v, %v
    %4:ptr<function, array<array<u32, 2>, 2>, read_write> = var
    %5:ptr<function, array<u32, 2>, read_write> = access %4, 0u
    store %5, %3
    %6:ptr<function, array<u32, 2>, read_write> = access %4, 1u
    store %6, %3
    %7:array<array<u32, 2>, 2> = load %4
    ret %7
  }
}
)";

    Run(PolyfillNestedAggregateConstruct);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_PolyfillNestedAggregateConstructTest, ThreeLevelNesting) {
    auto* s = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.Register("a"), ty.u32()},
                                                  {mod.symbols.Register("b"), ty.u32()},
                                              });
    auto* t = ty.Struct(mod.symbols.New("T"), {
                                                  {mod.symbols.Register("x"), s},
                                                  {mod.symbols.Register("y"), s},
                                              });
    auto* u = ty.Struct(mod.symbols.New("U"), {
                                                  {mod.symbols.Register("p"), t},
                                                  {mod.symbols.Register("q"), t},
                                              });
    auto* v = b.FunctionParam("v", ty.u32());
    auto* func = b.Function("foo", u);
    func->SetParams({v});

    b.Append(func->Block(), [&] {
        auto* sv = b.Construct(s, v, v);
        auto* tv = b.Construct(t, sv, sv);
        auto* uv = b.Construct(u, tv, tv);
        b.Return(func, uv);
    });

    auto* expect = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

T = struct @align(4) {
  x:S @offset(0)
  y:S @offset(8)
}

U = struct @align(4) {
  p:T @offset(0)
  q:T @offset(16)
}

%foo = func(%v:u32):U {
  $B1: {
    %3:S = construct %v, %v
    %4:ptr<function, T, read_write> = var
    %5:ptr<function, S, read_write> = access %4, 0u
    store %5, %3
    %6:ptr<function, S, read_write> = access %4, 1u
    store %6, %3
    %7:T = load %4
    %8:ptr<function, U, read_write> = var
    %9:ptr<function, T, read_write> = access %8, 0u
    store %9, %7
    %10:ptr<function, T, read_write> = access %8, 1u
    store %10, %7
    %11:U = load %8
    ret %11
  }
}
)";

    Run(PolyfillNestedAggregateConstruct);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_PolyfillNestedAggregateConstructTest, StructWithUnusedMember) {
    auto* inner = ty.Struct(mod.symbols.New("S"), {
                                                      {mod.symbols.Register("a"), ty.u32()},
                                                      {mod.symbols.Register("b"), ty.u32()},
                                                  });
    auto* outer = ty.Struct(mod.symbols.New("T"), {
                                                      {mod.symbols.Register("x"), inner},
                                                      {mod.symbols.Register("y"), inner},
                                                  });
    auto* s = b.FunctionParam("s", inner);
    auto* func = b.Function("foo", outer);
    func->SetParams({s});

    b.Append(func->Block(), [&] {
        auto* u = b.Construct(outer, s, b.Unused());
        b.Return(func, u);
    });

    auto* expect = R"(
S = struct @align(4) {
  a:u32 @offset(0)
  b:u32 @offset(4)
}

T = struct @align(4) {
  x:S @offset(0)
  y:S @offset(8)
}

%foo = func(%s:S):T {
  $B1: {
    %3:ptr<function, T, read_write> = var
    %4:ptr<function, S, read_write> = access %3, 0u
    store %4, %s
    %5:T = load %3
    ret %5
  }
}
)";

    Run(PolyfillNestedAggregateConstruct);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise
