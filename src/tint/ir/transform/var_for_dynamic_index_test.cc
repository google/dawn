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

#include "src/tint/ir/transform/var_for_dynamic_index.h"

#include <utility>

#include "src/tint/ir/transform/test_helper.h"
#include "src/tint/type/array.h"
#include "src/tint/type/matrix.h"
#include "src/tint/type/struct.h"

namespace tint::ir::transform {
namespace {

using namespace tint::number_suffixes;  // NOLINT

class IR_VarForDynamicIndexTest : public TransformTest {
  protected:
    const type::Type* ptr(const type::Type* elem) {
        return ty.pointer(elem, builtin::AddressSpace::kFunction, builtin::Access::kReadWrite);
    }
};

TEST_F(IR_VarForDynamicIndexTest, NoModify_ConstantIndex_ArrayValue) {
    auto* arr = b.FunctionParam(ty.array(ty.i32(), 4u));
    auto* func = b.CreateFunction("foo", ty.i32());
    func->SetParams(utils::Vector{arr});

    auto* access = b.Access(ty.i32(), arr, utils::Vector{b.Constant(1_i)});
    func->StartTarget()->Append(access);
    func->StartTarget()->Append(b.Return(func, utils::Vector{access}));
    mod.functions.Push(func);

    auto* expect = R"(
%foo = func(%2:array<i32, 4>):i32 -> %b1 {
  %b1 = block {
    %3:i32 = access %2, 1i
    ret %3
  }
}
)";

    Run<VarForDynamicIndex>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VarForDynamicIndexTest, NoModify_ConstantIndex_MatrixValue) {
    auto* mat = b.FunctionParam(ty.mat2x2(ty.f32()));
    auto* func = b.CreateFunction("foo", ty.f32());
    func->SetParams(utils::Vector{mat});

    auto* access = b.Access(ty.f32(), mat, utils::Vector{b.Constant(1_i), b.Constant(0_i)});
    func->StartTarget()->Append(access);
    func->StartTarget()->Append(b.Return(func, utils::Vector{access}));
    mod.functions.Push(func);

    auto* expect = R"(
%foo = func(%2:mat2x2<f32>):f32 -> %b1 {
  %b1 = block {
    %3:f32 = access %2, 1i, 0i
    ret %3
  }
}
)";

    Run<VarForDynamicIndex>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VarForDynamicIndexTest, NoModify_DynamicIndex_ArrayPointer) {
    auto* arr = b.FunctionParam(ptr(ty.array(ty.i32(), 4u)));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.CreateFunction("foo", ty.i32());
    func->SetParams(utils::Vector{arr, idx});

    auto* access = b.Access(ptr(ty.i32()), arr, utils::Vector{idx});
    auto* load = b.Load(access);
    func->StartTarget()->Append(access);
    func->StartTarget()->Append(load);
    func->StartTarget()->Append(b.Return(func, utils::Vector{load}));
    mod.functions.Push(func);

    auto* expect = R"(
%foo = func(%2:ptr<function, array<i32, 4>, read_write>, %3:i32):i32 -> %b1 {
  %b1 = block {
    %4:ptr<function, i32, read_write> = access %2, %3
    %5:i32 = load %4
    ret %5
  }
}
)";

    Run<VarForDynamicIndex>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VarForDynamicIndexTest, NoModify_DynamicIndex_MatrixPointer) {
    auto* mat = b.FunctionParam(ptr(ty.mat2x2(ty.f32())));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.CreateFunction("foo", ty.f32());
    func->SetParams(utils::Vector{mat, idx});

    auto* access = b.Access(ptr(ty.f32()), mat, utils::Vector{idx, idx});
    auto* load = b.Load(access);
    func->StartTarget()->Append(access);
    func->StartTarget()->Append(load);
    func->StartTarget()->Append(b.Return(func, utils::Vector{load}));
    mod.functions.Push(func);

    auto* expect = R"(
%foo = func(%2:ptr<function, mat2x2<f32>, read_write>, %3:i32):f32 -> %b1 {
  %b1 = block {
    %4:ptr<function, f32, read_write> = access %2, %3, %3
    %5:f32 = load %4
    ret %5
  }
}
)";

    Run<VarForDynamicIndex>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VarForDynamicIndexTest, NoModify_DynamicIndex_VectorValue) {
    auto* vec = b.FunctionParam(ty.vec4(ty.f32()));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.CreateFunction("foo", ty.f32());
    func->SetParams(utils::Vector{vec, idx});

    auto* access = b.Access(ty.f32(), vec, utils::Vector{idx});
    func->StartTarget()->Append(access);
    func->StartTarget()->Append(b.Return(func, utils::Vector{access}));
    mod.functions.Push(func);

    auto* expect = R"(
%foo = func(%2:vec4<f32>, %3:i32):f32 -> %b1 {
  %b1 = block {
    %4:f32 = access %2, %3
    ret %4
  }
}
)";

    Run<VarForDynamicIndex>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VarForDynamicIndexTest, DynamicIndex_ArrayValue) {
    auto* arr = b.FunctionParam(ty.array(ty.i32(), 4u));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.CreateFunction("foo", ty.i32());
    func->SetParams(utils::Vector{arr, idx});

    auto* access = b.Access(ty.i32(), arr, utils::Vector{idx});
    func->StartTarget()->Append(access);
    func->StartTarget()->Append(b.Return(func, utils::Vector{access}));
    mod.functions.Push(func);

    auto* expect = R"(
%foo = func(%2:array<i32, 4>, %3:i32):i32 -> %b1 {
  %b1 = block {
    %4:ptr<function, array<i32, 4>, read_write> = var, %2
    %5:ptr<function, i32, read_write> = access %4, %3
    %6:i32 = load %5
    ret %6
  }
}
)";

    Run<VarForDynamicIndex>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VarForDynamicIndexTest, DynamicIndex_MatrixValue) {
    auto* arr = b.FunctionParam(ty.mat2x2(ty.f32()));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.CreateFunction("foo", ty.f32());
    func->SetParams(utils::Vector{arr, idx});

    auto* access = b.Access(ty.f32(), arr, utils::Vector{idx});
    func->StartTarget()->Append(access);
    func->StartTarget()->Append(b.Return(func, utils::Vector{access}));
    mod.functions.Push(func);

    auto* expect = R"(
%foo = func(%2:mat2x2<f32>, %3:i32):f32 -> %b1 {
  %b1 = block {
    %4:ptr<function, mat2x2<f32>, read_write> = var, %2
    %5:ptr<function, f32, read_write> = access %4, %3
    %6:f32 = load %5
    ret %6
  }
}
)";

    Run<VarForDynamicIndex>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VarForDynamicIndexTest, AccessChain) {
    auto* arr = b.FunctionParam(ty.array(ty.array(ty.array(ty.i32(), 4u), 4u), 4u));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.CreateFunction("foo", ty.i32());
    func->SetParams(utils::Vector{arr, idx});

    auto* access = b.Access(ty.i32(), arr, utils::Vector{idx, b.Constant(1_u), idx});
    func->StartTarget()->Append(access);
    func->StartTarget()->Append(b.Return(func, utils::Vector{access}));
    mod.functions.Push(func);

    auto* expect = R"(
%foo = func(%2:array<array<array<i32, 4>, 4>, 4>, %3:i32):i32 -> %b1 {
  %b1 = block {
    %4:ptr<function, array<array<array<i32, 4>, 4>, 4>, read_write> = var, %2
    %5:ptr<function, i32, read_write> = access %4, %3, 1u, %3
    %6:i32 = load %5
    ret %6
  }
}
)";

    Run<VarForDynamicIndex>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VarForDynamicIndexTest, AccessChain_SkipConstantIndices) {
    auto* arr = b.FunctionParam(ty.array(ty.array(ty.array(ty.i32(), 4u), 4u), 4u));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.CreateFunction("foo", ty.i32());
    func->SetParams(utils::Vector{arr, idx});

    auto* access = b.Access(ty.i32(), arr, utils::Vector{b.Constant(1_u), b.Constant(2_u), idx});
    func->StartTarget()->Append(access);
    func->StartTarget()->Append(b.Return(func, utils::Vector{access}));
    mod.functions.Push(func);

    auto* expect = R"(
%foo = func(%2:array<array<array<i32, 4>, 4>, 4>, %3:i32):i32 -> %b1 {
  %b1 = block {
    %4:array<i32, 4> = access %2, 1u, 2u
    %5:ptr<function, array<i32, 4>, read_write> = var, %4
    %6:ptr<function, i32, read_write> = access %5, %3
    %7:i32 = load %6
    ret %7
  }
}
)";

    Run<VarForDynamicIndex>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VarForDynamicIndexTest, AccessChain_SkipConstantIndices_Interleaved) {
    auto* arr = b.FunctionParam(ty.array(ty.array(ty.array(ty.array(ty.i32(), 4u), 4u), 4u), 4u));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.CreateFunction("foo", ty.i32());
    func->SetParams(utils::Vector{arr, idx});

    auto* access =
        b.Access(ty.i32(), arr, utils::Vector{b.Constant(1_u), idx, b.Constant(2_u), idx});
    func->StartTarget()->Append(access);
    func->StartTarget()->Append(b.Return(func, utils::Vector{access}));
    mod.functions.Push(func);

    auto* expect = R"(
%foo = func(%2:array<array<array<array<i32, 4>, 4>, 4>, 4>, %3:i32):i32 -> %b1 {
  %b1 = block {
    %4:array<array<array<i32, 4>, 4>, 4> = access %2, 1u
    %5:ptr<function, array<array<array<i32, 4>, 4>, 4>, read_write> = var, %4
    %6:ptr<function, i32, read_write> = access %5, %3, 2u, %3
    %7:i32 = load %6
    ret %7
  }
}
)";

    Run<VarForDynamicIndex>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VarForDynamicIndexTest, AccessChain_SkipConstantIndices_Struct) {
    auto* str_ty = ty.Get<type::Struct>(
        mod.symbols.Register("MyStruct"),
        utils::Vector{
            ty.Get<type::StructMember>(mod.symbols.Register("arr1"), ty.array(ty.f32(), 1024u), 0u,
                                       0u, 4u, 4096u, type::StructMemberAttributes{}),
            ty.Get<type::StructMember>(mod.symbols.Register("mat"), ty.mat4x4(ty.f32()), 1u, 4096u,
                                       16u, 64u, type::StructMemberAttributes{}),
            ty.Get<type::StructMember>(mod.symbols.Register("arr2"), ty.array(ty.f32(), 1024u), 2u,
                                       4160u, 4u, 4096u, type::StructMemberAttributes{}),
        },
        16u, 32u, 32u);
    auto* str_val = b.FunctionParam(str_ty);
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.CreateFunction("foo", ty.f32());
    func->SetParams(utils::Vector{str_val, idx});

    auto* access =
        b.Access(ty.f32(), str_val, utils::Vector{b.Constant(1_u), idx, b.Constant(0_u)});
    func->StartTarget()->Append(access);
    func->StartTarget()->Append(b.Return(func, utils::Vector{access}));
    mod.functions.Push(func);

    auto* expect = R"(
MyStruct = struct @align(16) {
  arr1:array<f32, 1024> @offset(0)
  mat:mat4x4<f32> @offset(4096)
  arr2:array<f32, 1024> @offset(4160)
}

%foo = func(%2:MyStruct, %3:i32):f32 -> %b1 {
  %b1 = block {
    %4:mat4x4<f32> = access %2, 1u
    %5:ptr<function, mat4x4<f32>, read_write> = var, %4
    %6:ptr<function, f32, read_write> = access %5, %3, 0u
    %7:f32 = load %6
    ret %7
  }
}
)";

    Run<VarForDynamicIndex>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VarForDynamicIndexTest, MultipleAccessesFromSameSource) {
    auto* arr = b.FunctionParam(ty.array(ty.i32(), 4u));
    auto* idx_a = b.FunctionParam(ty.i32());
    auto* idx_b = b.FunctionParam(ty.i32());
    auto* idx_c = b.FunctionParam(ty.i32());
    auto* func = b.CreateFunction("foo", ty.i32());
    func->SetParams(utils::Vector{arr, idx_a, idx_b, idx_c});

    auto* access_a = b.Access(ty.i32(), arr, utils::Vector{idx_a});
    auto* access_b = b.Access(ty.i32(), arr, utils::Vector{idx_b});
    auto* access_c = b.Access(ty.i32(), arr, utils::Vector{idx_c});
    func->StartTarget()->Append(access_a);
    func->StartTarget()->Append(access_b);
    func->StartTarget()->Append(access_c);
    func->StartTarget()->Append(b.Return(func, utils::Vector{access_c}));
    mod.functions.Push(func);

    auto* expect = R"(
%foo = func(%2:array<i32, 4>, %3:i32, %4:i32, %5:i32):i32 -> %b1 {
  %b1 = block {
    %6:ptr<function, array<i32, 4>, read_write> = var, %2
    %7:ptr<function, i32, read_write> = access %6, %3
    %8:i32 = load %7
    %9:ptr<function, i32, read_write> = access %6, %4
    %10:i32 = load %9
    %11:ptr<function, i32, read_write> = access %6, %5
    %12:i32 = load %11
    ret %12
  }
}
)";

    Run<VarForDynamicIndex>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_VarForDynamicIndexTest, MultipleAccessesFromSameSource_SkipConstantIndices) {
    auto* arr = b.FunctionParam(ty.array(ty.array(ty.array(ty.i32(), 4u), 4u), 4u));
    auto* idx_a = b.FunctionParam(ty.i32());
    auto* idx_b = b.FunctionParam(ty.i32());
    auto* idx_c = b.FunctionParam(ty.i32());
    auto* func = b.CreateFunction("foo", ty.i32());
    func->SetParams(utils::Vector{arr, idx_a, idx_b, idx_c});

    auto* access_a =
        b.Access(ty.i32(), arr, utils::Vector{b.Constant(1_u), b.Constant(2_u), idx_a});
    auto* access_b =
        b.Access(ty.i32(), arr, utils::Vector{b.Constant(1_u), b.Constant(2_u), idx_b});
    auto* access_c =
        b.Access(ty.i32(), arr, utils::Vector{b.Constant(1_u), b.Constant(2_u), idx_c});
    func->StartTarget()->Append(access_a);
    func->StartTarget()->Append(access_b);
    func->StartTarget()->Append(access_c);
    func->StartTarget()->Append(b.Return(func, utils::Vector{access_c}));
    mod.functions.Push(func);

    auto* expect = R"(
%foo = func(%2:array<array<array<i32, 4>, 4>, 4>, %3:i32, %4:i32, %5:i32):i32 -> %b1 {
  %b1 = block {
    %6:array<i32, 4> = access %2, 1u, 2u
    %7:ptr<function, array<i32, 4>, read_write> = var, %6
    %8:ptr<function, i32, read_write> = access %7, %3
    %9:i32 = load %8
    %10:ptr<function, i32, read_write> = access %7, %4
    %11:i32 = load %10
    %12:ptr<function, i32, read_write> = access %7, %5
    %13:i32 = load %12
    ret %13
  }
}
)";

    Run<VarForDynamicIndex>();

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::ir::transform
