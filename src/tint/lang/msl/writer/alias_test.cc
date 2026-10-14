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

#include "gmock/gmock.h"
#include "src/tint/lang/msl/writer/helper_test.h"

namespace tint::msl::writer {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class MslWriterAliasTest : public MslWriterTest {
  protected:
    void SetUp() override {
        mod.properties.Add(core::ir::Property::kAllowBufferTypes,
                           core::ir::Property::kAllow16BitFloats);
    }
};

TEST_F(MslWriterAliasTest, U32) {
    auto* v = b.Var("v", ty.ptr(storage, ty.buffer(32)));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.u32()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.u32()}, v, 0_u);
        b.Load(view);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  device array<uchar, 32>* v;
};
typedef uint __attribute__((__may_alias__)) tint_aliased_u32;

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device array<uchar, 32>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=v};
  device tint_aliased_u32* const v_1 = reinterpret_cast<device uint*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
}
)");
}

TEST_F(MslWriterAliasTest, I32) {
    auto* v = b.Var("v", ty.ptr(storage, ty.buffer(32)));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.i32()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.i32()}, v, 0_u);
        b.Load(view);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  device array<uchar, 32>* v;
};
typedef int __attribute__((__may_alias__)) tint_aliased_i32;

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device array<uchar, 32>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=v};
  device tint_aliased_i32* const v_1 = reinterpret_cast<device int*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
}
)");
}

TEST_F(MslWriterAliasTest, F32) {
    auto* v = b.Var("v", ty.ptr(storage, ty.buffer(32)));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.f32()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.f32()}, v, 0_u);
        b.Load(view);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  device array<uchar, 32>* v;
};
typedef float __attribute__((__may_alias__)) tint_aliased_f32;

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device array<uchar, 32>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=v};
  device tint_aliased_f32* const v_1 = reinterpret_cast<device float*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
}
)");
}

TEST_F(MslWriterAliasTest, F16) {
    auto* v = b.Var("v", ty.ptr(storage, ty.buffer(32)));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.f16()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.f16()}, v, 0_u);
        b.Load(view);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  device array<uchar, 32>* v;
};
typedef half __attribute__((__may_alias__)) tint_aliased_f16;

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device array<uchar, 32>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=v};
  device tint_aliased_f16* const v_1 = reinterpret_cast<device half*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
}
)");
}

TEST_F(MslWriterAliasTest, Vec2i) {
    auto* v = b.Var("v", ty.ptr(storage, ty.buffer(32)));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.vec2i()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.vec2i()}, v, 0_u);
        b.LoadVectorElement(view, 0_u);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  device array<uchar, 32>* v;
};
typedef int2 __attribute__((__may_alias__)) tint_aliased_vec2_i32;

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device array<uchar, 32>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=v};
  device tint_aliased_vec2_i32* const v_1 = reinterpret_cast<device int2*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
}
)");
}

TEST_F(MslWriterAliasTest, Vec3h) {
    auto* v = b.Var("v", ty.ptr(storage, ty.buffer(32)));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.vec3h()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.vec3h()}, v, 0_u);
        b.LoadVectorElement(view, 0_u);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  device array<uchar, 32>* v;
};
typedef packed_half3 __attribute__((__may_alias__)) tint_aliased_packed_vec3_f16;

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device array<uchar, 32>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=v};
  device tint_aliased_packed_vec3_f16* const v_1 = reinterpret_cast<device packed_half3*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
}
)");
}

TEST_F(MslWriterAliasTest, Vec4f) {
    auto* v = b.Var("v", ty.ptr(storage, ty.buffer(32)));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.vec4f()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.vec4f()}, v, 0_u);
        b.LoadVectorElement(view, 0_u);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  device array<uchar, 32>* v;
};
typedef float4 __attribute__((__may_alias__)) tint_aliased_vec4_f32;

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device array<uchar, 32>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=v};
  device tint_aliased_vec4_f32* const v_1 = reinterpret_cast<device float4*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
}
)");
}

TEST_F(MslWriterAliasTest, Array_U32) {
    auto* v = b.Var("v", ty.ptr(storage, ty.buffer(32)));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view =
            b.CallExplicit(ty.ptr(storage, ty.array(ty.u32(), 4)), core::BuiltinFn::kBufferView,
                           Vector<core::ir::TemplateParameter, 1>{ty.array(ty.u32(), 4)}, v, 0_u);
        b.Load(b.Access(ty.ptr(storage, ty.u32()), view, 0_u));
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  device array<uchar, 32>* v;
};
typedef array<uint, 4> __attribute__((__may_alias__)) tint_aliased_array_u32_4;

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device array<uchar, 32>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=v};
  device tint_aliased_array_u32_4* const v_1 = reinterpret_cast<device array<uint, 4>*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
}
)");
}

TEST_F(MslWriterAliasTest, RuntimeArray_Vec4f) {
    auto* v = b.Var("v", ty.ptr(storage, ty.buffer(32)));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(
            ty.ptr(storage, ty.runtime_array(ty.vec4f())), core::BuiltinFn::kBufferArrayView,
            Vector<core::ir::TemplateParameter, 1>{ty.runtime_array(ty.vec4f())}, v, 0_u, 32_u);
        b.LoadVectorElement(b.Access(ty.ptr(storage, ty.vec4f()), view, 0_u), 0_u);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  device array<uchar, 32>* v;
};
typedef array<float4, 1> __attribute__((__may_alias__)) tint_aliased_array_vec4_f32;

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device array<uchar, 32>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=v};
  device tint_aliased_array_vec4_f32* const v_1 = reinterpret_cast<device array<float4, 1>*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
}
)");
}

TEST_F(MslWriterAliasTest, Struct) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.u32()},
                                              });
    auto* v = b.Var("v", ty.ptr(workgroup, ty.buffer(32)));
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(workgroup, S), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{S}, v, 0_u);
        b.Load(b.Access(ty.ptr(workgroup, ty.u32()), view, 0_u));
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  threadgroup array<uchar, 32>* v;
};

struct S {
  /* 0x0000 */ uint a;
};
typedef S __attribute__((__may_alias__)) tint_aliased_S;

struct tint_symbol_1 {
  array<uchar, 32> tint_symbol;
};

void entry_inner(uint tint_local_index, tint_module_vars_struct tint_module_vars) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint const v_1 = idx;
      if ((v_1 >= 32u)) {
        break;
      }
      (*tint_module_vars.v)[v_1] = 0u;
      {
        idx = (idx + 1u);
      }
    }
  }
  (threadgroup_barrier(mem_flags::mem_threadgroup));
  threadgroup tint_aliased_S* const v_2 = reinterpret_cast<threadgroup S*>(reinterpret_cast<threadgroup char*>(tint_module_vars.v) + 0u);
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(uint tint_local_index [[thread_index_in_threadgroup]], threadgroup tint_symbol_1* v_3 [[threadgroup(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=(&(*v_3).tint_symbol)};
  (entry_inner(tint_local_index, tint_module_vars));
}
)");
}

TEST_F(MslWriterAliasTest, Array_Struct) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.u32()},
                                              });
    auto* v = b.Var("v", ty.ptr(workgroup, ty.buffer(32)));
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(workgroup, ty.array(S, 2)), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.array(S, 2)}, v, 0_u);
        b.Load(b.Access(ty.ptr(workgroup, ty.u32()), view, 0_u, 0_u));
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  threadgroup array<uchar, 32>* v;
};

struct S {
  /* 0x0000 */ uint a;
};
typedef array<S, 2> __attribute__((__may_alias__)) tint_aliased_array_S_2;

struct tint_symbol_1 {
  array<uchar, 32> tint_symbol;
};

void entry_inner(uint tint_local_index, tint_module_vars_struct tint_module_vars) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint const v_1 = idx;
      if ((v_1 >= 32u)) {
        break;
      }
      (*tint_module_vars.v)[v_1] = 0u;
      {
        idx = (idx + 1u);
      }
    }
  }
  (threadgroup_barrier(mem_flags::mem_threadgroup));
  threadgroup tint_aliased_array_S_2* const v_2 = reinterpret_cast<threadgroup array<S, 2>*>(reinterpret_cast<threadgroup char*>(tint_module_vars.v) + 0u);
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(uint tint_local_index [[thread_index_in_threadgroup]], threadgroup tint_symbol_1* v_3 [[threadgroup(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=(&(*v_3).tint_symbol)};
  (entry_inner(tint_local_index, tint_module_vars));
}
)");
}

TEST_F(MslWriterAliasTest, NameCollisions) {
    auto* S1 = ty.Struct(mod.symbols.New("tint_aliased_u32"), {
                                                                  {mod.symbols.New("a"), ty.u32()},
                                                              });
    auto* S2 = ty.Struct(mod.symbols.New("tint_aliased_u32_1"),
                         {
                             {mod.symbols.New("tint_aliased_u32_2"), ty.u32()},
                         });
    auto* v = b.Var("v", ty.ptr(storage, ty.buffer(32)));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);
    auto* v2 = b.Var("v2", ty.ptr(storage, S1));
    v2->SetBindingPoint(0, 1);
    mod.root_block->Append(v2);
    auto* v3 = b.Var("v3", ty.ptr(storage, S2));
    v3->SetBindingPoint(0, 2);
    mod.root_block->Append(v3);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        b.Let("a", v2);
        b.Let("b", v3);
        auto* view = b.CallExplicit(ty.ptr(storage, ty.u32()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.u32()}, v, 0_u);
        b.Load(view);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_aliased_u32 {
  /* 0x0000 */ uint a;
};

struct tint_aliased_u32_1 {
  /* 0x0000 */ uint tint_aliased_u32_2;
};

struct tint_module_vars_struct {
  device array<uchar, 32>* v;
  device tint_aliased_u32* v2;
  device tint_aliased_u32_1* v3;
};
typedef uint __attribute__((__may_alias__)) tint_aliased_u32_3;

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device array<uchar, 32>* v [[buffer(0)]], device tint_aliased_u32* v2 [[buffer(1)]], device tint_aliased_u32_1* v3 [[buffer(2)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=v, .v2=v2, .v3=v3};
  device tint_aliased_u32* const a = tint_module_vars.v2;
  device tint_aliased_u32_1* const b = tint_module_vars.v3;
  device tint_aliased_u32_3* const v_1 = reinterpret_cast<device uint*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
}
)");
}

TEST_F(MslWriterAliasTest, Let_Fanout) {
    auto* S = ty.Struct(mod.symbols.New("S"), {
                                                  {mod.symbols.New("a"), ty.u32()},
                                              });
    auto* v = b.Var("v", ty.ptr(workgroup, ty.buffer(32)));
    mod.root_block->Append(v);

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(workgroup, ty.array(S, 2)), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.array(S, 2)}, v, 0_u);
        auto* l1 = b.Let("l1", view);
        auto* a1 = b.Access(ty.ptr(workgroup, S), l1, 0_u);
        auto* a2 = b.Access(ty.ptr(workgroup, S), l1, 1_u);
        auto* l2 = b.Let("l2", a1);
        auto* l3 = b.Let("l3", a2);
        b.Let("ld1", b.Load(b.Access(ty.ptr(workgroup, ty.u32()), l2, 0_u)));
        b.Let("ld2", b.Load(b.Access(ty.ptr(workgroup, ty.u32()), l3, 0_u)));
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl, MetalHeader() + R"(
struct tint_module_vars_struct {
  threadgroup array<uchar, 32>* v;
};

struct S {
  /* 0x0000 */ uint a;
};
typedef array<S, 2> __attribute__((__may_alias__)) tint_aliased_array_S_2;
typedef S __attribute__((__may_alias__)) tint_aliased_S;

struct tint_symbol_1 {
  array<uchar, 32> tint_symbol;
};

void entry_inner(uint tint_local_index, tint_module_vars_struct tint_module_vars) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint const v_1 = idx;
      if ((v_1 >= 32u)) {
        break;
      }
      (*tint_module_vars.v)[v_1] = 0u;
      {
        idx = (idx + 1u);
      }
    }
  }
  (threadgroup_barrier(mem_flags::mem_threadgroup));
  threadgroup tint_aliased_array_S_2* const v_2 = reinterpret_cast<threadgroup array<S, 2>*>(reinterpret_cast<threadgroup char*>(tint_module_vars.v) + 0u);
  threadgroup tint_aliased_array_S_2* const l1 = v_2;
  threadgroup tint_aliased_S* const l2 = (&(*l1)[0u]);
  threadgroup tint_aliased_S* const l3 = (&(*l1)[1u]);
  uint const ld1 = (*l2).a;
  uint const ld2 = (*l3).a;
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(uint tint_local_index [[thread_index_in_threadgroup]], threadgroup tint_symbol_1* v_3 [[threadgroup(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=(&(*v_3).tint_symbol)};
  (entry_inner(tint_local_index, tint_module_vars));
}
)");
}

TEST_F(MslWriterAliasTest, FunctionParameter) {
    auto* v = b.Var("v", ty.ptr(storage, ty.buffer(32)));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* other = b.FunctionParam("other", ty.u32());
    auto* p = b.FunctionParam("p", ty.ptr(storage, ty.u32()));
    foo->SetParams({other, p});
    b.Append(foo->Block(), [&] {
        b.Let("ld", b.Load(p));
        b.Return(foo);
    });

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.u32()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.u32()}, v, 0_u);
        b.Call(ty.void_(), foo, 0_u, view);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl,
              MetalHeader() + R"(typedef uint __attribute__((__may_alias__)) tint_aliased_u32;

struct tint_module_vars_struct {
  device array<uchar, 32>* v;
};

void foo(uint other, device tint_aliased_u32* const p) {
  uint const ld = (*p);
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device array<uchar, 32>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=v};
  device tint_aliased_u32* const v_1 = reinterpret_cast<device uint*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
  (foo(0u, v_1));
}
)");
}

TEST_F(MslWriterAliasTest, FunctionParameter_Nested) {
    auto* v = b.Var("v", ty.ptr(storage, ty.buffer(32)));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* bar = b.Function("bar", ty.void_());
    auto* p1 = b.FunctionParam("p1", ty.ptr(storage, ty.u32()));
    auto* p2 = b.FunctionParam("p2", ty.ptr(storage, ty.f32()));
    bar->SetParams({p1, p2});
    b.Append(bar->Block(), [&] {
        b.Let("ld1", b.Load(p1));
        b.Let("ld2", b.Load(p2));
        b.Return(bar);
    });

    auto* foo = b.Function("foo", ty.void_());
    auto* other = b.FunctionParam("other", ty.u32());
    auto* p = b.FunctionParam("p", ty.ptr(storage, ty.u32()));
    foo->SetParams({other, p});
    b.Append(foo->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.f32()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.f32()}, v, 0_u);
        b.Call(ty.void_(), bar, p, view);
        b.Return(foo);
    });

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.u32()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.u32()}, v, 0_u);
        b.Call(ty.void_(), foo, 0_u, view);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl,
              MetalHeader() + R"(typedef uint __attribute__((__may_alias__)) tint_aliased_u32;
typedef float __attribute__((__may_alias__)) tint_aliased_f32;

struct tint_module_vars_struct {
  device array<uchar, 32>* v;
};

void bar(device tint_aliased_u32* const p1, device tint_aliased_f32* const p2) {
  uint const ld1 = (*p1);
  float const ld2 = (*p2);
}

void foo(uint other, device tint_aliased_u32* const p, tint_module_vars_struct tint_module_vars) {
  device tint_aliased_f32* const v_1 = reinterpret_cast<device float*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
  (bar(p, v_1));
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device array<uchar, 32>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=v};
  device tint_aliased_u32* const v_2 = reinterpret_cast<device uint*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
  (foo(0u, v_2, tint_module_vars));
}
)");
}

TEST_F(MslWriterAliasTest, FunctionParameter_LetFanout) {
    auto* v = b.Var("v", ty.ptr(storage, ty.buffer(32)));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* foo = b.Function("foo", ty.void_());
    auto* other = b.FunctionParam("other", ty.u32());
    auto* p = b.FunctionParam("p", ty.ptr(storage, ty.u32()));
    foo->SetParams({other, p});
    b.Append(foo->Block(), [&] {
        auto* l = b.Let("l", p);
        b.Let("ld1", b.Load(l));
        l = b.Let("l2", p);
        b.Let("ld2", b.Load(l));
        b.Return(foo);
    });

    auto* ep = b.ComputeFunction("entry");
    b.Append(ep->Block(), [&] {
        auto* view = b.CallExplicit(ty.ptr(storage, ty.u32()), core::BuiltinFn::kBufferView,
                                    Vector<core::ir::TemplateParameter, 1>{ty.u32()}, v, 0_u);
        b.Call(ty.void_(), foo, 0_u, view);
        b.Return(ep);
    });

    auto result = Generate();
    ASSERT_EQ(result, Success) << result.Failure() << output_.msl;
    EXPECT_EQ(output_.msl,
              MetalHeader() + R"(typedef uint __attribute__((__may_alias__)) tint_aliased_u32;

struct tint_module_vars_struct {
  device array<uchar, 32>* v;
};

void foo(uint other, device tint_aliased_u32* const p) {
  device tint_aliased_u32* const l = p;
  uint const ld1 = (*l);
  device tint_aliased_u32* const l2 = p;
  uint const ld2 = (*l2);
}

[[max_total_threads_per_threadgroup(1)]]
kernel void entry(device array<uchar, 32>* v [[buffer(0)]]) {
  tint_module_vars_struct const tint_module_vars = tint_module_vars_struct{.v=v};
  device tint_aliased_u32* const v_1 = reinterpret_cast<device uint*>(reinterpret_cast<device char*>(tint_module_vars.v) + 0u);
  (foo(0u, v_1));
}
)");
}

}  // namespace
}  // namespace tint::msl::writer
