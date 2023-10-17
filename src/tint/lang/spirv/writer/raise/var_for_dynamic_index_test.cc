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

#include "src/tint/lang/spirv/writer/raise/var_for_dynamic_index.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/struct.h"

namespace tint::spirv::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using SpirvWriter_VarForDynamicIndexTest = core::ir::transform::TransformTest;

TEST_F(SpirvWriter_VarForDynamicIndexTest, NoModify_ConstantIndex_ArrayValue) {
    auto* arr = b.FunctionParam(ty.array<i32, 4u>());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.i32(), arr, 1_i));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:array<i32, 4>):i32 -> %b1 {
  %b1 = block {
    %3:i32 = access %2, 1i
    ret %3
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, NoModify_ConstantIndex_MatrixValue) {
    auto* mat = b.FunctionParam(ty.mat2x2<f32>());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({mat});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.f32(), mat, 1_i, 0_i));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:mat2x2<f32>):f32 -> %b1 {
  %b1 = block {
    %3:f32 = access %2, 1i, 0i
    ret %3
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, NoModify_DynamicIndex_ArrayPointer) {
    auto* arr = b.FunctionParam(ty.ptr<function, array<i32, 4u>>());
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.ptr<function, i32>(), arr, idx));
    auto* load = block->Append(b.Load(access));
    block->Append(b.Return(func, load));

    auto* expect = R"(
%foo = func(%2:ptr<function, array<i32, 4>, read_write>, %3:i32):i32 -> %b1 {
  %b1 = block {
    %4:ptr<function, i32, read_write> = access %2, %3
    %5:i32 = load %4
    ret %5
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, NoModify_DynamicIndex_MatrixPointer) {
    auto* mat = b.FunctionParam(ty.ptr<function, mat2x2<f32>>());
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({mat, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.ptr<function, vec2<f32>>(), mat, idx));
    auto* load = block->Append(b.LoadVectorElement(access, idx));
    block->Append(b.Return(func, load));

    auto* expect = R"(
%foo = func(%2:ptr<function, mat2x2<f32>, read_write>, %3:i32):f32 -> %b1 {
  %b1 = block {
    %4:ptr<function, vec2<f32>, read_write> = access %2, %3
    %5:f32 = load_vector_element %4, %3
    ret %5
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, NoModify_DynamicIndex_VectorValue) {
    auto* vec = b.FunctionParam(ty.vec4<f32>());
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({vec, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.f32(), vec, idx));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:vec4<f32>, %3:i32):f32 -> %b1 {
  %b1 = block {
    %4:f32 = access %2, %3
    ret %4
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, DynamicIndex_ArrayValue) {
    auto* arr = b.FunctionParam(ty.array<i32, 4u>());
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.i32(), arr, idx));
    block->Append(b.Return(func, access));

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

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, DynamicIndex_MatrixValue) {
    auto* mat = b.FunctionParam(ty.mat2x2<f32>());
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.vec2<f32>());
    func->SetParams({mat, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.vec2<f32>(), mat, idx));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:mat2x2<f32>, %3:i32):vec2<f32> -> %b1 {
  %b1 = block {
    %4:ptr<function, mat2x2<f32>, read_write> = var, %2
    %5:ptr<function, vec2<f32>, read_write> = access %4, %3
    %6:vec2<f32> = load %5
    ret %6
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, DynamicIndex_VectorValue) {
    auto* mat = b.FunctionParam(ty.mat2x2<f32>());
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({mat, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.f32(), mat, idx, idx));
    block->Append(b.Return(func, access));

    auto* expect = R"(
%foo = func(%2:mat2x2<f32>, %3:i32):f32 -> %b1 {
  %b1 = block {
    %4:ptr<function, mat2x2<f32>, read_write> = var, %2
    %5:ptr<function, vec2<f32>, read_write> = access %4, %3
    %6:f32 = load_vector_element %5, %3
    ret %6
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, AccessChain) {
    auto* arr = b.FunctionParam(ty.array(ty.array(ty.array<i32, 4u>(), 4u), 4u));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.i32(), arr, idx, 1_u, idx));
    block->Append(b.Return(func, access));

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

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, AccessChain_SkipConstantIndices) {
    auto* arr = b.FunctionParam(ty.array(ty.array(ty.array<i32, 4u>(), 4u), 4u));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.i32(), arr, 1_u, 2_u, idx));
    block->Append(b.Return(func, access));

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

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, AccessChain_SkipConstantIndices_Interleaved) {
    auto* arr = b.FunctionParam(ty.array(ty.array(ty.array(ty.array<i32, 4u>(), 4u), 4u), 4u));
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.i32(), arr, 1_u, idx, 2_u, idx));
    block->Append(b.Return(func, access));

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

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, AccessChain_SkipConstantIndices_Struct) {
    auto* str_ty = ty.Struct(mod.symbols.Register("MyStruct"),
                             {
                                 {mod.symbols.Register("arr1"), ty.array<f32, 1024>()},
                                 {mod.symbols.Register("mat"), ty.mat4x4<f32>()},
                                 {mod.symbols.Register("arr2"), ty.array<f32, 1024>()},
                             });
    auto* str_val = b.FunctionParam(str_ty);
    auto* idx = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.f32());
    func->SetParams({str_val, idx});

    auto* block = func->Block();
    auto* access = block->Append(b.Access(ty.f32(), str_val, 1_u, idx, 0_u));
    block->Append(b.Return(func, access));

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
    %6:ptr<function, vec4<f32>, read_write> = access %5, %3
    %7:f32 = load_vector_element %6, 0u
    ret %7
  }
}
)";

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, MultipleAccessesFromSameSource) {
    auto* arr = b.FunctionParam(ty.array<i32, 4u>());
    auto* idx_a = b.FunctionParam(ty.i32());
    auto* idx_b = b.FunctionParam(ty.i32());
    auto* idx_c = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx_a, idx_b, idx_c});

    auto* block = func->Block();
    block->Append(b.Access(ty.i32(), arr, idx_a));
    block->Append(b.Access(ty.i32(), arr, idx_b));
    auto* access_c = block->Append(b.Access(ty.i32(), arr, idx_c));
    block->Append(b.Return(func, access_c));

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

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvWriter_VarForDynamicIndexTest, MultipleAccessesFromSameSource_SkipConstantIndices) {
    auto* arr = b.FunctionParam(ty.array(ty.array(ty.array<i32, 4u>(), 4u), 4u));
    auto* idx_a = b.FunctionParam(ty.i32());
    auto* idx_b = b.FunctionParam(ty.i32());
    auto* idx_c = b.FunctionParam(ty.i32());
    auto* func = b.Function("foo", ty.i32());
    func->SetParams({arr, idx_a, idx_b, idx_c});

    auto* block = func->Block();
    block->Append(b.Access(ty.i32(), arr, 1_u, 2_u, idx_a));
    block->Append(b.Access(ty.i32(), arr, 1_u, 2_u, idx_b));
    auto* access_c = block->Append(b.Access(ty.i32(), arr, 1_u, 2_u, idx_c));
    block->Append(b.Return(func, access_c));

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

    Run(VarForDynamicIndex);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::writer::raise
