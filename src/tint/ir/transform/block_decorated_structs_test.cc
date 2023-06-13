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

#include "src/tint/ir/transform/block_decorated_structs.h"

#include <utility>

#include "src/tint/ir/transform/test_helper.h"
#include "src/tint/type/array.h"
#include "src/tint/type/pointer.h"
#include "src/tint/type/struct.h"

namespace tint::ir::transform {
namespace {

using IR_BlockDecoratedStructsTest = TransformTest;

using namespace tint::number_suffixes;  // NOLINT

TEST_F(IR_BlockDecoratedStructsTest, NoRootBlock) {
    auto* func = b.Function("foo", ty.void_());
    func->StartTarget()->Append(b.Return(func));
    mod.functions.Push(func);

    auto* expect = R"(
%foo = func():void -> %b1 {
  %b1 = block {
    ret
  }
}
)";

    Run<BlockDecoratedStructs>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, Scalar_Uniform) {
    auto* buffer = b.Var(ty.ptr<uniform, i32>());
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", ty.i32());
    auto* block = func->StartTarget();
    auto* load = block->Append(b.Load(buffer));
    block->Append(b.Return(func, load));
    mod.functions.Push(func);

    auto* expect = R"(
tint_symbol_1 = struct @align(4), @block {
  tint_symbol:i32 @offset(0)
}

# Root block
%b1 = block {
  %1:ptr<uniform, tint_symbol_1, read_write> = var @binding_point(0, 0)
}

%foo = func():i32 -> %b2 {
  %b2 = block {
    %3:ptr<uniform, i32, read_write> = access %1, 0u
    %4:i32 = load %3
    ret %4
  }
}
)";

    Run<BlockDecoratedStructs>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, Scalar_Storage) {
    auto* buffer = b.Var(ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    func->StartTarget()->Append(b.Store(buffer, 42_i));
    func->StartTarget()->Append(b.Return(func));
    mod.functions.Push(func);

    auto* expect = R"(
tint_symbol_1 = struct @align(4), @block {
  tint_symbol:i32 @offset(0)
}

# Root block
%b1 = block {
  %1:ptr<storage, tint_symbol_1, read_write> = var @binding_point(0, 0)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %3:ptr<storage, i32, read_write> = access %1, 0u
    store %3, 42i
    ret
  }
}
)";

    Run<BlockDecoratedStructs>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, RuntimeArray) {
    auto* buffer = b.Var(ty.ptr(storage, ty.runtime_array(ty.i32()), builtin::Access::kReadWrite));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* func = b.Function("foo", ty.void_());
    auto* block = func->StartTarget();
    auto* access = block->Append(b.Access(ty.ptr<storage, i32>(), buffer, 1_u));
    block->Append(b.Store(access, 42_i));
    block->Append(b.Return(func));
    mod.functions.Push(func);

    auto* expect = R"(
tint_symbol_1 = struct @align(4), @block {
  tint_symbol:array<i32> @offset(0)
}

# Root block
%b1 = block {
  %1:ptr<storage, tint_symbol_1, read_write> = var @binding_point(0, 0)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %3:ptr<storage, array<i32>, read_write> = access %1, 0u
    %4:ptr<storage, i32, read_write> = access %3, 1u
    store %4, 42i
    ret
  }
}
)";

    Run<BlockDecoratedStructs>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, RuntimeArray_InStruct) {
    utils::Vector<const type::StructMember*, 4> members;
    members.Push(ty.Get<type::StructMember>(mod.symbols.New(), ty.i32(), 0u, 0u, 4u, 4u,
                                            type::StructMemberAttributes{}));
    members.Push(ty.Get<type::StructMember>(mod.symbols.New(), ty.runtime_array(ty.i32()), 1u, 4u,
                                            4u, 4u, type::StructMemberAttributes{}));
    auto* structure = ty.Get<type::Struct>(mod.symbols.New(), members, 4u, 8u, 8u);

    auto* buffer = b.Var(ty.ptr(storage, structure, builtin::Access::kReadWrite));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* i32_ptr = ty.ptr<storage, i32>();

    auto* func = b.Function("foo", ty.void_());
    auto* block = func->StartTarget();
    auto* val_ptr = block->Append(b.Access(i32_ptr, buffer, 0_u));
    auto* load = block->Append(b.Load(val_ptr));
    auto* elem_ptr = block->Append(b.Access(i32_ptr, buffer, 1_u, 3_u));
    block->Append(b.Store(elem_ptr, load));
    block->Append(b.Return(func));
    mod.functions.Push(func);

    auto* expect = R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:i32 @offset(0)
  tint_symbol_1:array<i32> @offset(4)
}

tint_symbol_3 = struct @align(4), @block {
  tint_symbol:i32 @offset(0)
  tint_symbol_1:array<i32> @offset(4)
}

# Root block
%b1 = block {
  %1:ptr<storage, tint_symbol_3, read_write> = var @binding_point(0, 0)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %3:ptr<storage, i32, read_write> = access %1, 0u
    %4:i32 = load %3
    %5:ptr<storage, i32, read_write> = access %1, 1u, 3u
    store %5, %4
    ret
  }
}
)";

    Run<BlockDecoratedStructs>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, StructUsedElsewhere) {
    utils::Vector<const type::StructMember*, 4> members;
    members.Push(ty.Get<type::StructMember>(mod.symbols.New(), ty.i32(), 0u, 0u, 4u, 4u,
                                            type::StructMemberAttributes{}));
    members.Push(ty.Get<type::StructMember>(mod.symbols.New(), ty.i32(), 1u, 4u, 4u, 4u,
                                            type::StructMemberAttributes{}));
    auto* structure = ty.Get<type::Struct>(mod.symbols.New(), members, 4u, 8u, 8u);

    auto* buffer = b.Var(ty.ptr(storage, structure, builtin::Access::kReadWrite));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

    auto* private_var =
        b.Var(ty.ptr(builtin::AddressSpace::kPrivate, structure, builtin::Access::kReadWrite));
    b.RootBlock()->Append(private_var);

    auto* func = b.Function("foo", ty.void_());
    func->StartTarget()->Append(b.Store(buffer, private_var));
    func->StartTarget()->Append(b.Return(func));
    mod.functions.Push(func);

    auto* expect = R"(
tint_symbol_2 = struct @align(4) {
  tint_symbol:i32 @offset(0)
  tint_symbol_1:i32 @offset(4)
}

tint_symbol_4 = struct @align(4), @block {
  tint_symbol_3:tint_symbol_2 @offset(0)
}

# Root block
%b1 = block {
  %1:ptr<storage, tint_symbol_4, read_write> = var @binding_point(0, 0)
  %2:ptr<private, tint_symbol_2, read_write> = var
}

%foo = func():void -> %b2 {
  %b2 = block {
    %4:ptr<storage, tint_symbol_2, read_write> = access %1, 0u
    store %4, %2
    ret
  }
}
)";

    Run<BlockDecoratedStructs>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_BlockDecoratedStructsTest, MultipleBuffers) {
    auto* buffer_a = b.Var(ty.ptr<storage, i32>());
    auto* buffer_b = b.Var(ty.ptr<storage, i32>());
    auto* buffer_c = b.Var(ty.ptr<storage, i32>());
    buffer_a->SetBindingPoint(0, 0);
    buffer_b->SetBindingPoint(0, 1);
    buffer_c->SetBindingPoint(0, 2);
    auto* root = b.RootBlock();
    root->Append(buffer_a);
    root->Append(buffer_b);
    root->Append(buffer_c);

    auto* func = b.Function("foo", ty.void_());
    auto* block = func->StartTarget();
    auto* load_b = block->Append(b.Load(buffer_b));
    auto* load_c = block->Append(b.Load(buffer_c));
    block->Append(b.Store(buffer_a, b.Add(ty.i32(), load_b, load_c)));
    block->Append(b.Return(func));
    mod.functions.Push(func);

    auto* expect = R"(
tint_symbol_1 = struct @align(4), @block {
  tint_symbol:i32 @offset(0)
}

tint_symbol_3 = struct @align(4), @block {
  tint_symbol_2:i32 @offset(0)
}

tint_symbol_5 = struct @align(4), @block {
  tint_symbol_4:i32 @offset(0)
}

# Root block
%b1 = block {
  %1:ptr<storage, tint_symbol_1, read_write> = var @binding_point(0, 0)
  %2:ptr<storage, tint_symbol_3, read_write> = var @binding_point(0, 1)
  %3:ptr<storage, tint_symbol_5, read_write> = var @binding_point(0, 2)
}

%foo = func():void -> %b2 {
  %b2 = block {
    %5:ptr<storage, i32, read_write> = access %2, 0u
    %6:i32 = load %5
    %7:ptr<storage, i32, read_write> = access %3, 0u
    %8:i32 = load %7
    %9:ptr<storage, i32, read_write> = access %1, 0u
    store %9, %10
    ret
  }
}
)";

    Run<BlockDecoratedStructs>();

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::ir::transform
