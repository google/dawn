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

#include <string>
#include <tuple>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/utils/text/string.h"

namespace tint::core::ir {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

class IR_ValidatorTest : public IRTestHelper {
  public:
    /// Builds and returns a basic 'compute' entry point function, named @p name
    Function* ComputeEntryPoint(const std::string& name = "f") { return b.ComputeFunction(name); }

    /// Builds and returns a basic 'fragment' entry point function, named @p name
    Function* FragmentEntryPoint(const std::string& name = "f") {
        return b.Function(name, ty.void_(), Function::PipelineStage::kFragment);
    }

    /// Builds and returns a basic 'vertex' entry point function, named @p name
    Function* VertexEntryPoint(const std::string& name = "f") {
        auto* f = b.Function(name, ty.vec4<f32>(), Function::PipelineStage::kVertex);
        IOAttributes attr;
        attr.builtin = BuiltinValue::kPosition;
        f->SetReturnAttributes(attr);
        return f;
    }

    /// Adds to a function an input param named @p name of type @p type, and decorated with @p
    /// builtin
    void AddBuiltinParam(Function* func,
                         const std::string& name,
                         BuiltinValue builtin,
                         const core::type::Type* type) {
        IOAttributes attr;
        attr.builtin = builtin;
        auto* p = b.FunctionParam(name, type);
        p->SetAttributes(attr);
        func->AppendParam(p);
    }

    /// Adds to a function an return value of type @p type with attributes @p attr.
    /// If there is an already existing non-structured return, both values are moved into a
    /// structured return using @p name as the name.
    /// If there is an already existing structured return, then this ICEs, since that is beyond the
    /// scope of this implementation.
    void AddReturn(Function* func,
                   const std::string& name,
                   const core::type::Type* type,
                   const IOAttributes& attr = {}) {
        if (func->ReturnType()->Is<core::type::Struct>()) {
            TINT_ICE() << "AddReturn does not support adding to structured returns";
        }

        if (func->ReturnType() == ty.void_()) {
            func->SetReturnAttributes(attr);
            func->SetReturnType(type);
            return;
        }

        std::string old_name =
            func->ReturnAttributes().builtin == BuiltinValue::kPosition ? "pos" : "old_ret";
        auto* str_ty =
            ty.Struct(mod.symbols.New("OutputStruct"),
                      {
                          {mod.symbols.New(old_name), func->ReturnType(), func->ReturnAttributes()},
                          {mod.symbols.New(name), type, attr},
                      });

        func->SetReturnAttributes({});
        func->SetReturnType(str_ty);
    }

    /// Adds to a function an return value of type @p type, and decorated with @p builtin.
    /// See @ref AddReturn for more details
    void AddBuiltinReturn(Function* func,
                          const std::string& name,
                          BuiltinValue builtin,
                          const core::type::Type* type) {
        IOAttributes attr;
        attr.builtin = builtin;
        AddReturn(func, name, type, attr);
    }
};

TEST_F(IR_ValidatorTest, RootBlock_Var) {
    mod.root_block->Append(b.Var(ty.ptr<private_, i32>()));
    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, RootBlock_NonVar) {
    auto* l = b.Loop();
    l->Body()->Append(b.Continue(l));

    mod.root_block->Append(l);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:3 error: loop: root block: invalid instruction: tint::core::ir::Loop
  loop [b: $B2] {  # loop_1
  ^^^^^^^^^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  loop [b: $B2] {  # loop_1
    $B2: {  # body
      continue  # -> $B3
    }
  }
}

)");
}

TEST_F(IR_ValidatorTest, RootBlock_Let) {
    mod.root_block->Append(b.Let("a", 1_f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:12 error: let: root block: invalid instruction: tint::core::ir::Let
  %a:f32 = let 1.0f
           ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %a:f32 = let 1.0f
}

)");
}

TEST_F(IR_ValidatorTest, RootBlock_LetWithAllowModuleScopeLets) {
    mod.root_block->Append(b.Let("a", 1_f));

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowModuleScopeLets});
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, RootBlock_Construct) {
    mod.root_block->Append(b.Construct(ty.vec2<f32>(), 1_f, 2_f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:18 error: construct: root block: invalid instruction: tint::core::ir::Construct
  %1:vec2<f32> = construct 1.0f, 2.0f
                 ^^^^^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:vec2<f32> = construct 1.0f, 2.0f
}

)");
}

TEST_F(IR_ValidatorTest, RootBlock_ConstructWithAllowModuleScopeLets) {
    mod.root_block->Append(b.Construct(ty.vec2<f32>(), 1_f, 2_f));

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowModuleScopeLets});
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, RootBlock_VarBlockMismatch) {
    auto* var = b.Var(ty.ptr<private_, i32>());
    mod.root_block->Append(var);

    auto* f = b.Function("f", ty.void_());
    f->Block()->Append(b.Return(f));
    var->SetBlock(f->Block());

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:38 error: var: instruction in root block does not have root block as parent
  %1:ptr<private, i32, read_write> = var
                                     ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<private, i32, read_write> = var
}

%f = func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function) {
    auto* f = b.Function("my_func", ty.void_());

    f->SetParams({b.FunctionParam(ty.i32()), b.FunctionParam(ty.f32())});
    f->Block()->Append(b.Return(f));

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, Function_Duplicate) {
    auto* f = b.Function("my_func", ty.void_());
    // Function would auto-push by the builder, so this adds a duplicate
    mod.functions.Push(f);

    f->SetParams({b.FunctionParam(ty.i32()), b.FunctionParam(ty.f32())});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: function %my_func added to module multiple times
%my_func = func(%2:i32, %3:f32):void {
^^^^^^^^

note: # Disassembly
%my_func = func(%2:i32, %3:f32):void {
  $B1: {
    ret
  }
}
%my_func = func(%2:i32, %3:f32):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_MultinBlock) {
    auto* f = b.Function("my_func", ty.void_());
    f->SetBlock(b.MultiInBlock());
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: root block for function cannot be a multi-in block
%my_func = func():void {
^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_DeadParameter) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.f32());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    p->Destroy();

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:17 error: destroyed parameter found in function parameter list
%my_func = func(%my_param:f32):void {
                ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func(%my_param:f32):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_ParameterWithNullFunction) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.f32());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    p->SetFunction(nullptr);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:17 error: function parameter has nullptr parent function
%my_func = func(%my_param:f32):void {
                ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func(%my_param:f32):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_ParameterUsedInMultipleFunctions) {
    auto* p = b.FunctionParam("my_param", ty.f32());
    auto* f1 = b.Function("my_func1", ty.void_());
    auto* f2 = b.Function("my_func2", ty.void_());
    f1->SetParams({p});
    f2->SetParams({p});
    f1->Block()->Append(b.Return(f1));
    f2->Block()->Append(b.Return(f2));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:18 error: function parameter has incorrect parent function
%my_func1 = func(%my_param:f32):void {
                 ^^^^^^^^^^^^^

:6:1 note: parent function declared here
%my_func2 = func(%my_param:f32):void {
^^^^^^^^^

note: # Disassembly
%my_func1 = func(%my_param:f32):void {
  $B1: {
    ret
  }
}
%my_func2 = func(%my_param:f32):void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_ParameterWithNullType) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", nullptr);
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:17 error: function parameter has nullptr type
%my_func = func(%my_param:undef):void {
                ^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func(%my_param:undef):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_ParameterDuplicated) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.u32());
    f->SetParams({p, p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:17 error: function parameter is not unique
%my_func = func(%my_param:u32%my_param:u32):void {
                ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func(%my_param:u32%my_param:u32):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Param_BothLocationAndBuiltin) {
    auto* f = FragmentEntryPoint("my_func");

    auto* p = b.FunctionParam("my_param", ty.vec4<f32>());
    IOAttributes attr;
    attr.builtin = BuiltinValue::kPosition;
    attr.location = 0;
    p->SetAttributes(attr);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:27 error: a builtin and location cannot be both declared for a param
%my_func = @fragment func(%my_param:vec4<f32> [@location(0), @position]):void {
                          ^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = @fragment func(%my_param:vec4<f32> [@location(0), @position]):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Param_Struct_BothLocationAndBuiltin) {
    auto* f = FragmentEntryPoint("my_func");

    IOAttributes attr;
    attr.builtin = BuiltinValue::kPosition;
    attr.location = 0;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.vec4<f32>(), attr},
                                               });
    auto* p = b.FunctionParam("my_param", str_ty);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:27 error: a builtin and location cannot be both declared for a struct member
%my_func = @fragment func(%my_param:MyStruct):void {
                          ^^^^^^^^^^^^^^^^^^

note: # Disassembly
MyStruct = struct @align(16) {
  a:vec4<f32> @offset(0), @location(0), @builtin(position)
}

%my_func = @fragment func(%my_param:MyStruct):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_ParameterWithConstructibleType) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.u32());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_ParameterWithPointerType) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.ptr<function, i32>());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_ParameterWithTextureType) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.external_texture());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_ParameterWithSamplerType) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.sampler());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_ParameterWithVoidType) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.void_());
    f->SetParams({p});
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:1:17 error: function parameter type must be constructible, a pointer, a texture, or a sampler
%my_func = func(%my_param:void):void {
                ^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func(%my_param:void):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Param_InvariantWithPosition) {
    auto* f = b.Function("my_func", ty.void_(), Function::PipelineStage::kFragment);

    auto* p = b.FunctionParam("my_param", ty.vec4<f32>());
    IOAttributes attr;
    attr.builtin = BuiltinValue::kPosition;
    attr.invariant = true;
    p->SetAttributes(attr);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_Param_InvariantWithoutPosition) {
    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", ty.vec4<f32>());
    IOAttributes attr;
    attr.invariant = true;
    p->SetAttributes(attr);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:1:17 error: invariant can only decorate a param iff it is also decorated with position
%my_func = func(%my_param:vec4<f32> [@invariant]):void {
                ^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func(%my_param:vec4<f32> [@invariant]):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Param_Struct_InvariantWithPosition) {
    auto* f = b.Function("my_func", ty.void_(), Function::PipelineStage::kFragment);

    IOAttributes attr;
    attr.invariant = true;
    attr.builtin = BuiltinValue::kPosition;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), ty.vec4<f32>(), attr},
                                               });
    auto* p = b.FunctionParam("my_param", str_ty);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_Param_Struct_InvariantWithoutPosition) {
    IOAttributes attr;
    attr.invariant = true;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), ty.vec4<f32>(), attr},
                                               });

    auto* f = b.Function("my_func", ty.void_());
    auto* p = b.FunctionParam("my_param", str_ty);
    f->SetParams({p});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:17 error: invariant can only decorate a param member iff it is also decorated with position
%my_func = func(%my_param:MyStruct):void {
                ^^^^^^^^^^^^^^^^^^

note: # Disassembly
MyStruct = struct @align(16) {
  pos:vec4<f32> @offset(0), @invariant
}

%my_func = func(%my_param:MyStruct):void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Return_BothLocationAndBuiltin) {
    auto* f = VertexEntryPoint("my_func");
    IOAttributes attr;
    attr.builtin = BuiltinValue::kPosition;
    attr.location = 0;
    f->SetReturnAttributes(attr);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: a builtin and location cannot be both declared for a function return
%my_func = @vertex func():vec4<f32> [@location(0), @position] {
^^^^^^^^

note: # Disassembly
%my_func = @vertex func():vec4<f32> [@location(0), @position] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Return_Struct_BothLocationAndBuiltin) {
    IOAttributes attr;
    attr.builtin = BuiltinValue::kPosition;
    attr.location = 0;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.vec4<f32>(), attr},
                                               });
    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kVertex);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:1 error: a builtin and location cannot be both declared for a struct member
%my_func = @vertex func():MyStruct {
^^^^^^^^

note: # Disassembly
MyStruct = struct @align(16) {
  a:vec4<f32> @offset(0), @location(0), @builtin(position)
}

%my_func = @vertex func():MyStruct {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Return_NonVoid_MissingLocationAndBuiltin) {
    auto* f = b.Function("my_func", ty.f32(), Function::PipelineStage::kFragment);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:1:1 error: a non-void return for an entry point must have a builtin or location decoration
%my_func = @fragment func():f32 {
^^^^^^^^

note: # Disassembly
%my_func = @fragment func():f32 {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Return_NonVoid_Struct_MissingLocationAndBuiltin) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.f32(), {}},
                                                          });

    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kFragment);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:1 error: members of struct used for returns of entry points must have a builtin or location decoration
%my_func = @fragment func():MyStruct {
^^^^^^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:f32 @offset(0)
}

%my_func = @fragment func():MyStruct {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Return_InvariantWithPosition) {
    IOAttributes attr;
    attr.builtin = BuiltinValue::kPosition;
    attr.invariant = true;

    auto* f = b.Function("my_func", ty.vec4<f32>(), Function::PipelineStage::kVertex);
    f->SetReturnAttributes(attr);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_Return_InvariantWithoutPosition) {
    IOAttributes attr;
    attr.invariant = true;

    auto* f = b.Function("my_func", ty.vec4<f32>());
    f->SetReturnAttributes(attr);

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: invariant can only decorate outputs iff they are also position builtins
%my_func = func():vec4<f32> [@invariant] {
^^^^^^^^

note: # Disassembly
%my_func = func():vec4<f32> [@invariant] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Return_Struct_InvariantWithPosition) {
    IOAttributes attr;
    attr.invariant = true;
    attr.builtin = BuiltinValue::kPosition;
    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), ty.vec4<f32>(), attr},
                                               });

    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kVertex);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_Return_Struct_InvariantWithoutPosition) {
    IOAttributes attr;
    attr.invariant = true;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), ty.vec4<f32>(), attr},
                                               });

    auto* f = b.Function("my_func", str_ty);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:1 error: invariant can only decorate output members iff they are also position builtins
%my_func = func():MyStruct {
^^^^^^^^

note: # Disassembly
MyStruct = struct @align(16) {
  pos:vec4<f32> @offset(0), @invariant
}

%my_func = func():MyStruct {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_UnnamedEntryPoint) {
    auto* f = b.Function(ty.void_(), ir::Function::PipelineStage::kCompute);
    f->SetWorkgroupSize({b.Constant(1_u), b.Constant(1_u), b.Constant(1_u)});

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: entry points must have names
%1 = @compute @workgroup_size(1u, 1u, 1u) func():void {
^^

note: # Disassembly
%1 = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_NonConstructibleReturnType) {
    auto types = Vector<const core::type::Type*, 2>{
        ty.external_texture(),   ty.sampler(), ty.runtime_array(ty.f32()), ty.ptr<function, i32>(),
        ty.ref<function, u32>(),
    };

    for (auto t : types) {
        auto* f = b.Function(t);
        b.Append(f->Block(), [&] { b.Unreachable(); });
    }

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: function return type must be constructible
%1 = func():texture_external {
^^

:6:1 error: function return type must be constructible
%2 = func():sampler {
^^

:11:1 error: function return type must be constructible
%3 = func():array<f32> {
^^

:16:1 error: function return type must be constructible
%4 = func():ptr<function, i32, read_write> {
^^

:21:1 error: reference types are not permitted here
%5 = func():ref<function, u32, read_write> {
^^

:21:1 error: function return type must be constructible
%5 = func():ref<function, u32, read_write> {
^^

note: # Disassembly
%1 = func():texture_external {
  $B1: {
    unreachable
  }
}
%2 = func():sampler {
  $B2: {
    unreachable
  }
}
%3 = func():array<f32> {
  $B3: {
    unreachable
  }
}
%4 = func():ptr<function, i32, read_write> {
  $B4: {
    unreachable
  }
}
%5 = func():ref<function, u32, read_write> {
  $B5: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Compute_NonVoidReturn) {
    auto* f = b.Function("my_func", ty.f32(), core::ir::Function::PipelineStage::kCompute);
    f->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: compute entry point must not have a return type
%my_func = @compute @workgroup_size(1u, 1u, 1u) func():f32 {
^^^^^^^^

note: # Disassembly
%my_func = @compute @workgroup_size(1u, 1u, 1u) func():f32 {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_MissingOnCompute) {
    auto* f = b.Function("f", ty.void_(), Function::PipelineStage::kCompute);
    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: compute entry point requires @workgroup_size
%f = @compute func():void {
^^

note: # Disassembly
%f = @compute func():void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_NonCompute) {
    auto* f = FragmentEntryPoint();
    f->SetWorkgroupSize(b.Constant(1_u), b.Constant(1_u), b.Constant(1_u));

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: @workgroup_size only valid on compute entry point
%f = @fragment @workgroup_size(1u, 1u, 1u) func():void {
^^

note: # Disassembly
%f = @fragment @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_ParamUndefined) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({nullptr, b.Constant(2_u), b.Constant(3_u)});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: a @workgroup_size param is undefined or missing a type
%f = @compute @workgroup_size(undef, 2u, 3u) func():void {
^^

note: # Disassembly
%f = @compute @workgroup_size(undef, 2u, 3u) func():void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_ParamWrongType) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_f), b.Constant(2_u), b.Constant(3_u)});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: @workgroup_size params must be an i32 or u32
%f = @compute @workgroup_size(1.0f, 2u, 3u) func():void {
^^

note: # Disassembly
%f = @compute @workgroup_size(1.0f, 2u, 3u) func():void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_ParamsSameType) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(1_u), b.Constant(2_i), b.Constant(3_u)});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: @workgroup_size params must be all i32s or all u32s
%f = @compute @workgroup_size(1u, 2i, 3u) func():void {
^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 2i, 3u) func():void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_ParamsTooSmall) {
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({b.Constant(-1_i), b.Constant(2_i), b.Constant(3_i)});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: @workgroup_size params must be greater than 0
%f = @compute @workgroup_size(-1i, 2i, 3i) func():void {
^^

note: # Disassembly
%f = @compute @workgroup_size(-1i, 2i, 3i) func():void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_WorkgroupSize_OverrideWithoutAllowOverrides) {
    auto* o = b.Override(ty.u32());
    auto* f = ComputeEntryPoint();
    f->SetWorkgroupSize({o->Result(0), o->Result(0), o->Result(0)});

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:1:1 error: @workgroup_size param is not a constant value, and IR capability 'kAllowOverrides' is not set
%f = @compute @workgroup_size(%2, %2, %2) func():void {
^^

note: # Disassembly
%f = @compute @workgroup_size(%2, %2, %2) func():void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Vertex_BasicPosition) {
    auto* f = b.Function("my_func", ty.vec4<f32>(), Function::PipelineStage::kVertex);
    f->SetReturnBuiltin(BuiltinValue::kPosition);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_Vertex_StructPosition) {
    auto pos_ty = ty.vec4<f32>();
    auto pos_attr = IOAttributes();
    pos_attr.builtin = BuiltinValue::kPosition;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), pos_ty, pos_attr},
                                               });

    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kVertex);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_Vertex_StructPositionAndClipDistances) {
    auto pos_ty = ty.vec4<f32>();
    auto pos_attr = IOAttributes();
    pos_attr.builtin = BuiltinValue::kPosition;

    auto clip_ty = ty.array<f32, 4>();
    auto clip_attr = IOAttributes();
    clip_attr.builtin = BuiltinValue::kClipDistances;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("pos"), pos_ty, pos_attr},
                                                   {mod.symbols.New("clip"), clip_ty, clip_attr},
                                               });

    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kVertex);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Function_Vertex_StructOnlyClipDistances) {
    auto clip_ty = ty.array<f32, 4>();
    auto clip_attr = IOAttributes();
    clip_attr.builtin = BuiltinValue::kClipDistances;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("clip"), clip_ty, clip_attr},
                                               });

    auto* f = b.Function("my_func", str_ty, Function::PipelineStage::kVertex);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:1 error: position must be declared for vertex entry point output
%my_func = @vertex func():MyStruct {
^^^^^^^^

note: # Disassembly
MyStruct = struct @align(4) {
  clip:array<f32, 4> @offset(0), @builtin(clip_distances)
}

%my_func = @vertex func():MyStruct {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Vertex_MissingPosition) {
    auto* f = b.Function("my_func", ty.vec4<f32>(), Function::PipelineStage::kVertex);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: position must be declared for vertex entry point output
%my_func = @vertex func():vec4<f32> {
^^^^^^^^

note: # Disassembly
%my_func = @vertex func():vec4<f32> {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_NonFragment_BoolInput) {
    auto* f = VertexEntryPoint();
    f->AppendParam(b.FunctionParam("invalid", ty.bool_()));
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:19 error: entry point params can only be a bool for fragment shaders
%f = @vertex func(%invalid:bool):vec4<f32> [@position] {
                  ^^^^^^^^^^^^^

note: # Disassembly
%f = @vertex func(%invalid:bool):vec4<f32> [@position] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_NonFragment_BoolOutput) {
    auto* f = VertexEntryPoint();
    AddReturn(f, "invalid", ty.bool_());
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:6:1 error: entry point return members can not be bool
%f = @vertex func():OutputStruct {
^^

note: # Disassembly
OutputStruct = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  invalid:bool @offset(16)
}

%f = @vertex func():OutputStruct {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Fragment_BoolInputWithoutFrontFacing) {
    auto* f = FragmentEntryPoint();
    f->AppendParam(b.FunctionParam("invalid", ty.bool_()));
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:1:21 error: fragment entry point params can only be a bool if decorated with @builtin(front_facing)
%f = @fragment func(%invalid:bool):void {
                    ^^^^^^^^^^^^^

note: # Disassembly
%f = @fragment func(%invalid:bool):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_Fragment_BoolOutput) {
    auto* f = FragmentEntryPoint();
    IOAttributes attr;
    attr.location = 0;
    AddReturn(f, "invalid", ty.bool_(), attr);
    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: entry point returns can not be bool
%f = @fragment func():bool [@location(0)] {
^^

note: # Disassembly
%f = @fragment func():bool [@location(0)] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_BoolOutput_via_MSV) {
    auto* f = ComputeEntryPoint();

    auto* v = b.Var(ty.ptr(AddressSpace::kOut, ty.bool_(), core::Access::kReadWrite));
    mod.root_block->Append(v);

    b.Append(f->Block(), [&] {
        b.Append(
            mod.CreateInstruction<ir::Store>(v->Result(0), b.Constant(b.ConstantValue(false))));
        b.Unreachable();
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:1 error: IO address space values referenced by shader entry points can only be bool if in the input space, used only by fragment shaders and decorated with @builtin(front_facing)
%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
^^

note: # Disassembly
$B1: {  # root
  %1:ptr<__out, bool, read_write> = var
}

%f = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    store %1, false
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Function_BoolInputWithoutFrontFacing_via_MSV) {
    auto* f = FragmentEntryPoint();

    auto* invalid = b.Var("invalid", AddressSpace::kIn, ty.bool_());
    mod.root_block->Append(invalid);

    b.Append(f->Block(), [&] {
        auto* l = b.Load(invalid);
        auto* v = b.Var("v", AddressSpace::kPrivate, ty.bool_());
        v->SetInitializer(l->Result(0));
        b.Unreachable();
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:1 error: input address space values referenced by fragment shaders can only be a bool if decorated with @builtin(front_facing)
%f = @fragment func():void {
^^

note: # Disassembly
$B1: {  # root
  %invalid:ptr<__in, bool, read> = var
}

%f = @fragment func():void {
  $B2: {
    %3:bool = load %invalid
    %v:ptr<private, bool, read_write> = var, %3
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_PointSize_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "size", BuiltinValue::kPointSize, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: __point_size must be used in a vertex shader entry point
%f = @fragment func():f32 [@__point_size] {
^^

note: # Disassembly
%f = @fragment func():f32 [@__point_size] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_PointSize_WrongIODirection) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "size", BuiltinValue::kPointSize, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:19 error: __point_size must be an output of a shader entry point
%f = @vertex func(%size:f32 [@__point_size]):vec4<f32> [@position] {
                  ^^^^^^^^^

note: # Disassembly
%f = @vertex func(%size:f32 [@__point_size]):vec4<f32> [@position] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_PointSize_WrongType) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "size", BuiltinValue::kPointSize, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:6:1 error: __point_size must be a f32
%f = @vertex func():OutputStruct {
^^

note: # Disassembly
OutputStruct = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  size:u32 @offset(16), @builtin(__point_size)
}

%f = @vertex func():OutputStruct {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_ClipDistances_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "distances", BuiltinValue::kClipDistances, ty.array<f32, 2>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: clip_distances must be used in a vertex shader entry point
%f = @fragment func():array<f32, 2> [@clip_distances] {
^^

note: # Disassembly
%f = @fragment func():array<f32, 2> [@clip_distances] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_ClipDistances_WrongIODirection) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "distances", BuiltinValue::kClipDistances, ty.array<f32, 2>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:19 error: clip_distances must be an output of a shader entry point
%f = @vertex func(%distances:array<f32, 2> [@clip_distances]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%f = @vertex func(%distances:array<f32, 2> [@clip_distances]):vec4<f32> [@position] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_ClipDistances_WrongType) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "distances", BuiltinValue::kClipDistances, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:6:1 error: clip_distances must be an array<f32, N>, where N <= 8
%f = @vertex func():OutputStruct {
^^

note: # Disassembly
OutputStruct = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  distances:f32 @offset(16), @builtin(clip_distances)
}

%f = @vertex func():OutputStruct {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_FragDepth_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "depth", BuiltinValue::kFragDepth, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:6:1 error: frag_depth must be used in a fragment shader entry point
%f = @vertex func():OutputStruct {
^^

note: # Disassembly
OutputStruct = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  depth:f32 @offset(16), @builtin(frag_depth)
}

%f = @vertex func():OutputStruct {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_FragDepth_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "depth", BuiltinValue::kFragDepth, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:21 error: frag_depth must be an output of a shader entry point
%f = @fragment func(%depth:f32 [@frag_depth]):void {
                    ^^^^^^^^^^

note: # Disassembly
%f = @fragment func(%depth:f32 [@frag_depth]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_FragDepth_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "depth", BuiltinValue::kFragDepth, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: frag_depth must be a f32
%f = @fragment func():u32 [@frag_depth] {
^^

note: # Disassembly
%f = @fragment func():u32 [@frag_depth] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_FrontFacing_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "facing", BuiltinValue::kFrontFacing, ty.bool_());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:19 error: front_facing must be used in a fragment shader entry point
%f = @vertex func(%facing:bool [@front_facing]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^

:1:19 error: entry point params can only be a bool for fragment shaders
%f = @vertex func(%facing:bool [@front_facing]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^

note: # Disassembly
%f = @vertex func(%facing:bool [@front_facing]):vec4<f32> [@position] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_FrontFacing_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "facing", BuiltinValue::kFrontFacing, ty.bool_());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: front_facing must be an input of a shader entry point
%f = @fragment func():bool [@front_facing] {
^^

note: # Disassembly
%f = @fragment func():bool [@front_facing] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_FrontFacing_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "facing", BuiltinValue::kFrontFacing, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:21 error: front_facing must be a bool
%f = @fragment func(%facing:u32 [@front_facing]):void {
                    ^^^^^^^^^^^

note: # Disassembly
%f = @fragment func(%facing:u32 [@front_facing]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_GlobalInvocationId_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "invocation", BuiltinValue::kGlobalInvocationId, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:21 error: global_invocation_id must be used in a compute shader entry point
%f = @fragment func(%invocation:vec3<u32> [@global_invocation_id]):void {
                    ^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%f = @fragment func(%invocation:vec3<u32> [@global_invocation_id]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_GlobalInvocationId_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "invocation", BuiltinValue::kGlobalInvocationId, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: global_invocation_id must be an input of a shader entry point
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@global_invocation_id] {
^^

:1:1 error: compute entry point must not have a return type
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@global_invocation_id] {
^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@global_invocation_id] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_GlobalInvocationId_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "invocation", BuiltinValue::kGlobalInvocationId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:48 error: global_invocation_id must be an vec3<u32>
%f = @compute @workgroup_size(1u, 1u, 1u) func(%invocation:u32 [@global_invocation_id]):void {
                                               ^^^^^^^^^^^^^^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 1u, 1u) func(%invocation:u32 [@global_invocation_id]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_InstanceIndex_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "instance", BuiltinValue::kInstanceIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:21 error: instance_index must be used in a vertex shader entry point
%f = @fragment func(%instance:u32 [@instance_index]):void {
                    ^^^^^^^^^^^^^

note: # Disassembly
%f = @fragment func(%instance:u32 [@instance_index]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_InstanceIndex_WrongIODirection) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "instance", BuiltinValue::kInstanceIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:6:1 error: instance_index must be an input of a shader entry point
%f = @vertex func():OutputStruct {
^^

note: # Disassembly
OutputStruct = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  instance:u32 @offset(16), @builtin(instance_index)
}

%f = @vertex func():OutputStruct {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_InstanceIndex_WrongType) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "instance", BuiltinValue::kInstanceIndex, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:19 error: instance_index must be an u32
%f = @vertex func(%instance:i32 [@instance_index]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^^

note: # Disassembly
%f = @vertex func(%instance:i32 [@instance_index]):vec4<f32> [@position] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationId_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kLocalInvocationId, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:21 error: local_invocation_id must be used in a compute shader entry point
%f = @fragment func(%id:vec3<u32> [@local_invocation_id]):void {
                    ^^^^^^^^^^^^^

note: # Disassembly
%f = @fragment func(%id:vec3<u32> [@local_invocation_id]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationId_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "id", BuiltinValue::kLocalInvocationId, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: local_invocation_id must be an input of a shader entry point
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@local_invocation_id] {
^^

:1:1 error: compute entry point must not have a return type
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@local_invocation_id] {
^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@local_invocation_id] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationId_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kLocalInvocationId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:48 error: local_invocation_id must be an vec3<u32>
%f = @compute @workgroup_size(1u, 1u, 1u) func(%id:u32 [@local_invocation_id]):void {
                                               ^^^^^^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 1u, 1u) func(%id:u32 [@local_invocation_id]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationIndex_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kLocalInvocationIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:21 error: local_invocation_index must be used in a compute shader entry point
%f = @fragment func(%index:u32 [@local_invocation_index]):void {
                    ^^^^^^^^^^

note: # Disassembly
%f = @fragment func(%index:u32 [@local_invocation_index]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationIndex_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "index", BuiltinValue::kLocalInvocationIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: local_invocation_index must be an input of a shader entry point
%f = @compute @workgroup_size(1u, 1u, 1u) func():u32 [@local_invocation_index] {
^^

:1:1 error: compute entry point must not have a return type
%f = @compute @workgroup_size(1u, 1u, 1u) func():u32 [@local_invocation_index] {
^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 1u, 1u) func():u32 [@local_invocation_index] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_LocalInvocationIndex_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kLocalInvocationIndex, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:48 error: local_invocation_index must be an u32
%f = @compute @workgroup_size(1u, 1u, 1u) func(%index:i32 [@local_invocation_index]):void {
                                               ^^^^^^^^^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 1u, 1u) func(%index:i32 [@local_invocation_index]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_NumWorkgroups_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "num", BuiltinValue::kNumWorkgroups, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:21 error: num_workgroups must be used in a compute shader entry point
%f = @fragment func(%num:vec3<u32> [@num_workgroups]):void {
                    ^^^^^^^^^^^^^^

note: # Disassembly
%f = @fragment func(%num:vec3<u32> [@num_workgroups]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_NumWorkgroups_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "num", BuiltinValue::kNumWorkgroups, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: num_workgroups must be an input of a shader entry point
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@num_workgroups] {
^^

:1:1 error: compute entry point must not have a return type
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@num_workgroups] {
^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@num_workgroups] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_NumWorkgroups_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "num", BuiltinValue::kNumWorkgroups, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:48 error: num_workgroups must be an vec3<u32>
%f = @compute @workgroup_size(1u, 1u, 1u) func(%num:u32 [@num_workgroups]):void {
                                               ^^^^^^^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 1u, 1u) func(%num:u32 [@num_workgroups]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_SampleIndex_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kSampleIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:19 error: sample_index must be used in a fragment shader entry point
%f = @vertex func(%index:u32 [@sample_index]):vec4<f32> [@position] {
                  ^^^^^^^^^^

note: # Disassembly
%f = @vertex func(%index:u32 [@sample_index]):vec4<f32> [@position] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_SampleIndex_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "index", BuiltinValue::kSampleIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: sample_index must be an input of a shader entry point
%f = @fragment func():u32 [@sample_index] {
^^

note: # Disassembly
%f = @fragment func():u32 [@sample_index] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_SampleIndex_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kSampleIndex, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:21 error: sample_index must be an u32
%f = @fragment func(%index:f32 [@sample_index]):void {
                    ^^^^^^^^^^

note: # Disassembly
%f = @fragment func(%index:f32 [@sample_index]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_VertexIndex_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kVertexIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:21 error: vertex_index must be used in a vertex shader entry point
%f = @fragment func(%index:u32 [@vertex_index]):void {
                    ^^^^^^^^^^

note: # Disassembly
%f = @fragment func(%index:u32 [@vertex_index]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_VertexIndex_WrongIODirection) {
    auto* f = VertexEntryPoint();
    AddBuiltinReturn(f, "index", BuiltinValue::kVertexIndex, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:6:1 error: vertex_index must be an input of a shader entry point
%f = @vertex func():OutputStruct {
^^

note: # Disassembly
OutputStruct = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  index:u32 @offset(16), @builtin(vertex_index)
}

%f = @vertex func():OutputStruct {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_VertexIndex_WrongType) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "index", BuiltinValue::kVertexIndex, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:19 error: vertex_index must be an u32
%f = @vertex func(%index:f32 [@vertex_index]):vec4<f32> [@position] {
                  ^^^^^^^^^^

note: # Disassembly
%f = @vertex func(%index:f32 [@vertex_index]):vec4<f32> [@position] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_WorkgroupId_WrongStage) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kWorkgroupId, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:21 error: workgroup_id must be used in a compute shader entry point
%f = @fragment func(%id:vec3<u32> [@workgroup_id]):void {
                    ^^^^^^^^^^^^^

note: # Disassembly
%f = @fragment func(%id:vec3<u32> [@workgroup_id]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_WorkgroupId_WrongIODirection) {
    // This will also trigger the compute entry points should have void returns check
    auto* f = ComputeEntryPoint();
    AddBuiltinReturn(f, "id", BuiltinValue::kWorkgroupId, ty.vec3<u32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: workgroup_id must be an input of a shader entry point
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@workgroup_id] {
^^

:1:1 error: compute entry point must not have a return type
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@workgroup_id] {
^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 1u, 1u) func():vec3<u32> [@workgroup_id] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_WorkgroupId_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kWorkgroupId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:48 error: workgroup_id must be an vec3<u32>
%f = @compute @workgroup_size(1u, 1u, 1u) func(%id:u32 [@workgroup_id]):void {
                                               ^^^^^^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 1u, 1u) func(%id:u32 [@workgroup_id]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_Position_WrongStage) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "pos", BuiltinValue::kPosition, ty.vec4<f32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:48 error: position must be used in a fragment or vertex shader entry point
%f = @compute @workgroup_size(1u, 1u, 1u) func(%pos:vec4<f32> [@position]):void {
                                               ^^^^^^^^^^^^^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 1u, 1u) func(%pos:vec4<f32> [@position]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_Position_WrongIODirectionForVertex) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "pos", BuiltinValue::kPosition, ty.vec4<f32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:19 error: position must be an output for a vertex entry point
%f = @vertex func(%pos:vec4<f32> [@position]):vec4<f32> [@position] {
                  ^^^^^^^^^^^^^^

note: # Disassembly
%f = @vertex func(%pos:vec4<f32> [@position]):vec4<f32> [@position] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_Position_WrongIODirectionForFragment) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "pos", BuiltinValue::kPosition, ty.vec4<f32>());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: position must be an input for a fragment entry point
%f = @fragment func():vec4<f32> [@position] {
^^

note: # Disassembly
%f = @fragment func():vec4<f32> [@position] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_Position_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "pos", BuiltinValue::kPosition, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:21 error: position must be an vec4<f32>
%f = @fragment func(%pos:f32 [@position]):void {
                    ^^^^^^^^

note: # Disassembly
%f = @fragment func(%pos:f32 [@position]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_SampleMask_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "mask", BuiltinValue::kSampleMask, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:19 error: sample_mask must be used in a fragment entry point
%f = @vertex func(%mask:u32 [@sample_mask]):vec4<f32> [@position] {
                  ^^^^^^^^^

note: # Disassembly
%f = @vertex func(%mask:u32 [@sample_mask]):vec4<f32> [@position] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_SampleMask_InputValid) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "mask", BuiltinValue::kSampleMask, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Builtin_SampleMask_OutputValid) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "mask", BuiltinValue::kSampleMask, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Builtin_SampleMask_WrongType) {
    auto* f = FragmentEntryPoint();
    AddBuiltinParam(f, "mask", BuiltinValue::kSampleMask, ty.f32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:21 error: sample_mask must be an u32
%f = @fragment func(%mask:f32 [@sample_mask]):void {
                    ^^^^^^^^^

note: # Disassembly
%f = @fragment func(%mask:f32 [@sample_mask]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupSize_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "size", BuiltinValue::kSubgroupSize, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:19 error: subgroup_size must be used in a compute or fragment shader entry point
%f = @vertex func(%size:u32 [@subgroup_size]):vec4<f32> [@position] {
                  ^^^^^^^^^

note: # Disassembly
%f = @vertex func(%size:u32 [@subgroup_size]):vec4<f32> [@position] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupSize_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "size", BuiltinValue::kSubgroupSize, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: subgroup_size must be an input of a shader entry point
%f = @fragment func():u32 [@subgroup_size] {
^^

note: # Disassembly
%f = @fragment func():u32 [@subgroup_size] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupSize_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "size", BuiltinValue::kSubgroupSize, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:48 error: subgroup_size must be an u32
%f = @compute @workgroup_size(1u, 1u, 1u) func(%size:i32 [@subgroup_size]):void {
                                               ^^^^^^^^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 1u, 1u) func(%size:i32 [@subgroup_size]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupInvocationId_WrongStage) {
    auto* f = VertexEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kSubgroupInvocationId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:1:19 error: subgroup_invocation_id must be used in a compute or fragment shader entry point
%f = @vertex func(%id:u32 [@subgroup_invocation_id]):vec4<f32> [@position] {
                  ^^^^^^^

note: # Disassembly
%f = @vertex func(%id:u32 [@subgroup_invocation_id]):vec4<f32> [@position] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupInvocationId_WrongIODirection) {
    auto* f = FragmentEntryPoint();
    AddBuiltinReturn(f, "id", BuiltinValue::kSubgroupInvocationId, ty.u32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:1 error: subgroup_invocation_id must be an input of a shader entry point
%f = @fragment func():u32 [@subgroup_invocation_id] {
^^

note: # Disassembly
%f = @fragment func():u32 [@subgroup_invocation_id] {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Builtin_SubgroupInvocationId_WrongType) {
    auto* f = ComputeEntryPoint();
    AddBuiltinParam(f, "id", BuiltinValue::kSubgroupInvocationId, ty.i32());

    b.Append(f->Block(), [&] { b.Unreachable(); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:1:48 error: subgroup_invocation_id must be an u32
%f = @compute @workgroup_size(1u, 1u, 1u) func(%id:i32 [@subgroup_invocation_id]):void {
                                               ^^^^^^^

note: # Disassembly
%f = @compute @workgroup_size(1u, 1u, 1u) func(%id:i32 [@subgroup_invocation_id]):void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToFunctionOutsideModule) {
    auto* f = b.Function("f", ty.void_());
    auto* g = b.Function("g", ty.void_());
    mod.functions.Pop();  // Remove g

    b.Append(f->Block(), [&] {
        b.Call(g);
        b.Return(f);
    });
    b.Append(g->Block(), [&] { b.Return(g); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:20 error: call: %g is not part of the module
    %2:void = call %g
                   ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:void = call %g
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToEntryPointFunction) {
    auto* f = b.Function("f", ty.void_());
    auto* g = ComputeEntryPoint("g");

    b.Append(f->Block(), [&] {
        b.Call(g);
        b.Return(f);
    });
    b.Append(g->Block(), [&] { b.Return(g); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:20 error: call: call target must not have a pipeline stage
    %2:void = call %g
                   ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:void = call %g
    ret
  }
}
%g = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToFunctionTooFewArguments) {
    auto* g = b.Function("g", ty.void_());
    g->SetParams({b.FunctionParam<i32>(), b.FunctionParam<i32>()});
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call(g, 42_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:20 error: call: function has 2 parameters, but call provides 1 arguments
    %5:void = call %g, 42i
                   ^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func(%2:i32, %3:i32):void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    %5:void = call %g, 42i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToFunctionTooManyArguments) {
    auto* g = b.Function("g", ty.void_());
    g->SetParams({b.FunctionParam<i32>(), b.FunctionParam<i32>()});
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call(g, 1_i, 2_i, 3_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:20 error: call: function has 2 parameters, but call provides 3 arguments
    %5:void = call %g, 1i, 2i, 3i
                   ^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func(%2:i32, %3:i32):void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    %5:void = call %g, 1i, 2i, 3i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToFunctionWrongArgType) {
    auto* g = b.Function("g", ty.void_());
    g->SetParams({b.FunctionParam<i32>(), b.FunctionParam<i32>(), b.FunctionParam<i32>()});
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call(g, 1_i, 2_f, 3_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:8:28 error: call: function parameter 1 is of type 'i32', but argument is of type 'f32'
    %6:void = call %g, 1i, 2.0f, 3i
                           ^^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func(%2:i32, %3:i32, %4:i32):void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    %6:void = call %g, 1i, 2.0f, 3i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToFunctionNullArg) {
    auto* g = b.Function("g", ty.void_());
    g->SetParams({b.FunctionParam<i32>()});
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Call(g, nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:24 error: call: operand is undefined
    %4:void = call %g, undef
                       ^^^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func(%2:i32):void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    %4:void = call %g, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToNullFunction) {
    auto* g = b.Function("g", ty.void_());
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(g);
        c->SetOperands(Vector{static_cast<ir::Value*>(nullptr)});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:20 error: call: operand is undefined
    %3:void = call undef
                   ^^^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func():void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    %3:void = call undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToFunctionNoResult) {
    auto* g = b.Function("g", ty.void_());
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(g);
        c->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:13 error: call: expected exactly 1 results, got 0
    undef = call %g
            ^^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func():void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    undef = call %g
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToFunctionNoOperands) {
    auto* g = b.Function("g", ty.void_());
    b.Append(g->Block(), [&] { b.Return(g); });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(g);
        c->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:15 error: call: expected at least 1 operands, got 0
    %3:void = call undef
              ^^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
%g = func():void {
  $B1: {
    ret
  }
}
%f = func():void {
  $B2: {
    %3:void = call undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToNonFunctionTarget) {
    auto* g = b.Function("g", ty.void_());
    mod.functions.Pop();  // Remove g, since it isn't actually going to be used, it is just needed
                          // to create the UserCall before mangling it

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(g);
        c->SetOperands(Vector{b.Value(0_i)});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:20 error: call: target not defined or not a function
    %2:void = call 0i
                   ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:void = call 0i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToBuiltinMissingResult) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(ty.f32(), BuiltinFn::kAbs, 1_f);
        c->SetResults(Vector{static_cast<ir::InstructionResult*>(nullptr)});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:13 error: abs: call to builtin does not have a return type
    undef = abs 1.0f
            ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    undef = abs 1.0f
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToBuiltinMismatchResultType) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Call(ty.f32(), BuiltinFn::kAbs, 1_f);
        c->Result(0)->SetType(ty.i32());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:14 error: abs: call result type does not match builtin return type
    %2:i32 = abs 1.0f
             ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:i32 = abs 1.0f
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, CallToBuiltinArgNullType) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(b.Constant(0_f));
        auto* load = b.Load(i);
        auto* load_ret = load->Result(0);
        b.Call(ty.f32(), BuiltinFn::kAbs, load_ret);
        load_ret->SetType(nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:5 error: load: result type is undefined
    %3:undef = load %i
    ^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:5:14 error: abs: argument to builtin has undefined type
    %4:f32 = abs %3
             ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %i:ptr<function, f32, read_write> = var, 0.0f
    %3:undef = load %i
    %4:f32 = abs %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Bitcast_MissingArg) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* bitcast = b.Bitcast(ty.i32(), 1_u);
        bitcast->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:14 error: bitcast: expected exactly 1 operands, got 0
    %2:i32 = bitcast
             ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:i32 = bitcast
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Bitcast_NullArg) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Bitcast(ty.i32(), nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:22 error: bitcast: operand is undefined
    %2:i32 = bitcast undef
                     ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:i32 = bitcast undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Bitcast_MissingResult) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* bitcast = b.Bitcast(ty.i32(), 1_u);
        bitcast->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:13 error: bitcast: expected exactly 1 results, got 0
    undef = bitcast 1u
            ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    undef = bitcast 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Bitcast_NullResult) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Bitcast(ty.i32(), 1_u);
        c->SetResults(Vector<InstructionResult*, 1>{nullptr});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: bitcast: result is undefined
    undef = bitcast 1u
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    undef = bitcast 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Construct_Struct_ZeroValue) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Struct_ValidArgs) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty, 1_i, 2_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Struct_UnusedArgs) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty, 1_i, b.Unused());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Construct_Struct_NotEnoughArgs) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty, 1_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:19 error: construct: structure has 2 members, but construct provides 1 arguments
    %2:MyStruct = construct 1i
                  ^^^^^^^^^

:7:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

%f = func():void {
  $B1: {
    %2:MyStruct = construct 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Construct_Struct_TooManyArgs) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty, 1_i, 2_u, 3_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:19 error: construct: structure has 2 members, but construct provides 3 arguments
    %2:MyStruct = construct 1i, 2u, 3i
                  ^^^^^^^^^

:7:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

%f = func():void {
  $B1: {
    %2:MyStruct = construct 1i, 2u, 3i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Construct_Struct_WrongArgType) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty, 1_i, 2_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:8:33 error: construct: structure member 1 is of type 'u32', but argument is of type 'i32'
    %2:MyStruct = construct 1i, 2i
                                ^^

:7:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

%f = func():void {
  $B1: {
    %2:MyStruct = construct 1i, 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Construct_NullArg) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Construct(str_ty, 1_i, nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:33 error: construct: operand is undefined
    %2:MyStruct = construct 1i, undef
                                ^^^^^

:7:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

%f = func():void {
  $B1: {
    %2:MyStruct = construct 1i, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Construct_NullResult) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Construct(str_ty, 1_i, 2_u);
        c->SetResults(Vector<ir::InstructionResult*, 1>{nullptr});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:5 error: construct: result is undefined
    undef = construct 1i, 2u
    ^^^^^

:7:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

%f = func():void {
  $B1: {
    undef = construct 1i, 2u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Construct_EmptyResult) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.u32()},
                                                          });

    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Construct(str_ty, 1_i, 2_u);
        c->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:13 error: construct: expected exactly 1 results, got 0
    undef = construct 1i, 2u
            ^^^^^^^^^

:7:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:u32 @offset(4)
}

%f = func():void {
  $B1: {
    undef = construct 1i, 2u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Convert_MissingArg) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Convert(ty.i32(), 1_f);
        c->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:14 error: convert: expected exactly 1 operands, got 0
    %2:i32 = convert
             ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:i32 = convert
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Convert_NullArg) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        b.Convert(ty.i32(), nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:22 error: convert: operand is undefined
    %2:i32 = convert undef
                     ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:i32 = convert undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Convert_MissingResult) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Convert(ty.i32(), 1_f);
        c->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:13 error: convert: expected exactly 1 results, got 0
    undef = convert 1.0f
            ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    undef = convert 1.0f
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Convert_NullResult) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {
        auto* c = b.Convert(ty.i32(), 1_f);
        c->SetResults(Vector<InstructionResult*, 1>{nullptr});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: convert: result is undefined
    undef = convert 1.0f
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    undef = convert 1.0f
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Discard_TooManyOperands) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* d = b.Discard();
        d->SetOperands(Vector{b.Value(0_i)});
        b.Return(func);
    });

    auto* ep = FragmentEntryPoint("ep");
    b.Append(ep->Block(), [&] {
        b.Call(func);
        b.Return(ep);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: discard: expected exactly 0 operands, got 1
    discard
    ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%foo = func():void {
  $B1: {
    discard
    ret
  }
}
%ep = @fragment func():void {
  $B2: {
    %3:void = call %foo
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Discard_TooManyResults) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        auto* d = b.Discard();
        d->SetResults(Vector{b.InstructionResult(ty.i32())});
        b.Return(func);
    });

    auto* ep = FragmentEntryPoint("ep");
    b.Append(ep->Block(), [&] {
        b.Call(func);
        b.Return(ep);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: discard: expected exactly 0 results, got 1
    discard
    ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%foo = func():void {
  $B1: {
    discard
    ret
  }
}
%ep = @fragment func():void {
  $B2: {
    %3:void = call %foo
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Discard_NotInFragment) {
    auto* func = b.Function("foo", ty.void_());
    b.Append(func->Block(), [&] {
        b.Discard();
        b.Return(func);
    });

    auto* ep = ComputeEntryPoint("ep");

    b.Append(ep->Block(), [&] {
        b.Call(func);
        b.Return(ep);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: discard: cannot be called in non-fragment end point
    discard
    ^^^^^^^

note: # Disassembly
%foo = func():void {
  $B1: {
    discard
    ret
  }
}
%ep = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %3:void = call %foo
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Block_NoTerminator) {
    b.Function("my_func", ty.void_());

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:3 error: block does not end in a terminator instruction
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
  }
}
)");
}

TEST_F(IR_ValidatorTest, Block_VarBlockMismatch) {
    auto* var = b.Var(ty.ptr<function, i32>());

    auto* f = b.Function("f", ty.void_());
    f->Block()->Append(var);
    f->Block()->Append(b.Return(f));

    auto* g = b.Function("g", ty.void_());
    g->Block()->Append(b.Return(g));

    var->SetBlock(g->Block());

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:41 error: var: block instruction does not have same block as parent
    %2:ptr<function, i32, read_write> = var
                                        ^^^

:2:3 note: in block
  $B1: {
  ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%f = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    ret
  }
}
%g = func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Block_DeadParameter) {
    auto* f = b.Function("my_func", ty.void_());

    auto* p = b.BlockParam("my_param", ty.f32());
    b.Append(f->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Initializer(), [&] { b.NextIteration(l, nullptr); });
        l->Body()->SetParams({p});
        b.Append(l->Body(), [&] { b.ExitLoop(l); });
        b.Return(f);
    });

    p->Destroy();

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:12 error: destroyed parameter found in block parameter list
      $B3 (%my_param:f32): {  # body
           ^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        next_iteration undef  # -> $B3
      }
      $B3 (%my_param:f32): {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Block_ParameterWithNullBlock) {
    auto* f = b.Function("my_func", ty.void_());

    auto* p = b.BlockParam("my_param", ty.f32());
    b.Append(f->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Initializer(), [&] { b.NextIteration(l, nullptr); });
        l->Body()->SetParams({p});
        b.Append(l->Body(), [&] { b.ExitLoop(l); });
        b.Return(f);
    });

    p->SetBlock(nullptr);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:12 error: block parameter has nullptr parent block
      $B3 (%my_param:f32): {  # body
           ^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        next_iteration undef  # -> $B3
      }
      $B3 (%my_param:f32): {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Block_ParameterUsedInMultipleBlocks) {
    auto* f = b.Function("my_func", ty.void_());

    auto* p = b.BlockParam("my_param", ty.f32());
    b.Append(f->Block(), [&] {
        auto* l = b.Loop();
        b.Append(l->Initializer(), [&] { b.NextIteration(l, nullptr); });
        l->Body()->SetParams({p});
        b.Append(l->Body(), [&] { b.Continue(l, p); });
        l->Continuing()->SetParams({p});
        b.Append(l->Continuing(), [&] { b.NextIteration(l, p); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:12 error: block parameter has incorrect parent block
      $B3 (%my_param:f32): {  # body
           ^^^^^^^^^

:10:7 note: parent block declared here
      $B4 (%my_param:f32): {  # continuing
      ^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration undef  # -> $B3
      }
      $B3 (%my_param:f32): {  # body
        continue %my_param  # -> $B4
      }
      $B4 (%my_param:f32): {  # continuing
        next_iteration %my_param  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_NoOperands) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        auto* access = b.Access(ty.f32(), obj, 0_i);
        access->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:14 error: access: expected at least 2 operands, got 0
    %3:f32 = access
             ^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:vec3<f32>):void {
  $B1: {
    %3:f32 = access
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_NoIndices) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:14 error: access: expected at least 2 operands, got 1
    %3:f32 = access %2
             ^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:vec3<f32>):void {
  $B1: {
    %3:f32 = access %2
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_NoResults) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        auto* access = b.Access(ty.f32(), obj, 0_i);
        access->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:13 error: access: expected exactly 1 results, got 0
    undef = access %2, 0i
            ^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:vec3<f32>):void {
  $B1: {
    undef = access %2, 0i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_NullObject) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), nullptr, 0_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:21 error: access: operand is undefined
    %2:f32 = access undef, 0u
                    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = access undef, 0u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_NullIndex) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:25 error: access: operand is undefined
    %3:f32 = access %2, undef
                        ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:vec3<f32>):void {
  $B1: {
    %3:f32 = access %2, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_NegativeIndex) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, -1_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:25 error: access: constant index must be positive, got -1
    %3:f32 = access %2, -1i
                        ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:vec3<f32>):void {
  $B1: {
    %3:f32 = access %2, -1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_OOB_Index_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.mat3x2<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 3_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:29 error: access: index out of bounds for type 'vec2<f32>'
    %3:f32 = access %2, 1u, 3u
                            ^^

:2:3 note: in block
  $B1: {
  ^^^

:3:29 note: acceptable range: [0..1]
    %3:f32 = access %2, 1u, 3u
                            ^^

note: # Disassembly
%my_func = func(%2:mat3x2<f32>):void {
  $B1: {
    %3:f32 = access %2, 1u, 3u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_OOB_Index_Ptr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, array<array<f32, 2>, 3>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u, 3_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:55 error: access: index out of bounds for type 'ptr<private, array<f32, 2>, read_write>'
    %3:ptr<private, f32, read_write> = access %2, 1u, 3u
                                                      ^^

:2:3 note: in block
  $B1: {
  ^^^

:3:55 note: acceptable range: [0..1]
    %3:ptr<private, f32, read_write> = access %2, 1u, 3u
                                                      ^^

note: # Disassembly
%my_func = func(%2:ptr<private, array<array<f32, 2>, 3>, read_write>):void {
  $B1: {
    %3:ptr<private, f32, read_write> = access %2, 1u, 3u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_StaticallyUnindexableType_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.f32());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:25 error: access: type 'f32' cannot be indexed
    %3:f32 = access %2, 1u
                        ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:f32):void {
  $B1: {
    %3:f32 = access %2, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_StaticallyUnindexableType_Ptr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:51 error: access: type 'ptr<private, f32, read_write>' cannot be indexed
    %3:ptr<private, f32, read_write> = access %2, 1u
                                                  ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<private, f32, read_write>):void {
  $B1: {
    %3:ptr<private, f32, read_write> = access %2, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_DynamicallyUnindexableType_Value) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.i32()},
                                                          });

    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(str_ty);
    auto* idx = b.FunctionParam(ty.i32());
    f->SetParams({obj, idx});

    b.Append(f->Block(), [&] {
        b.Access(ty.i32(), obj, idx);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:25 error: access: type 'MyStruct' cannot be dynamically indexed
    %4:i32 = access %2, %3
                        ^^

:7:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
}

%my_func = func(%2:MyStruct, %3:i32):void {
  $B1: {
    %4:i32 = access %2, %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_DynamicallyUnindexableType_Ptr) {
    auto* str_ty = ty.Struct(mod.symbols.New("MyStruct"), {
                                                              {mod.symbols.New("a"), ty.i32()},
                                                              {mod.symbols.New("b"), ty.i32()},
                                                          });

    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, read_write>(str_ty));
    auto* idx = b.FunctionParam(ty.i32());
    f->SetParams({obj, idx});

    b.Append(f->Block(), [&] {
        b.Access(ty.i32(), obj, idx);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:8:25 error: access: type 'ptr<private, MyStruct, read_write>' cannot be dynamically indexed
    %4:i32 = access %2, %3
                        ^^

:7:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:i32 @offset(0)
  b:i32 @offset(4)
}

%my_func = func(%2:ptr<private, MyStruct, read_write>, %3:i32):void {
  $B1: {
    %4:i32 = access %2, %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Type_Value_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.mat3x2<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.i32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:14 error: access: result of access chain is type 'f32' but instruction type is 'i32'
    %3:i32 = access %2, 1u, 1u
             ^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:mat3x2<f32>):void {
  $B1: {
    %3:i32 = access %2, 1u, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Type_Ptr_Ptr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, array<array<f32, 2>, 3>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, i32>(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:40 error: access: result of access chain is type 'ptr<private, f32, read_write>' but instruction type is 'ptr<private, i32, read_write>'
    %3:ptr<private, i32, read_write> = access %2, 1u, 1u
                                       ^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<private, array<array<f32, 2>, 3>, read_write>):void {
  $B1: {
    %3:ptr<private, i32, read_write> = access %2, 1u, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Type_Ptr_Value) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, array<array<f32, 2>, 3>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:14 error: access: result of access chain is type 'ptr<private, f32, read_write>' but instruction type is 'f32'
    %3:f32 = access %2, 1u, 1u
             ^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<private, array<array<f32, 2>, 3>, read_write>):void {
  $B1: {
    %3:f32 = access %2, 1u, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_IndexVectorPtr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, vec3<f32>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:25 error: access: cannot obtain address of vector element
    %3:f32 = access %2, 1u
                        ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<private, vec3<f32>, read_write>):void {
  $B1: {
    %3:f32 = access %2, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_IndexVectorPtr_WithCapability) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, vec3<f32>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowVectorElementPointer});
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Access_IndexVectorPtr_ViaMatrixPtr) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, mat3x2<f32>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:29 error: access: cannot obtain address of vector element
    %3:f32 = access %2, 1u, 1u
                            ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<private, mat3x2<f32>, read_write>):void {
  $B1: {
    %3:f32 = access %2, 1u, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_IndexVectorPtr_ViaMatrixPtr_WithCapability) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<private_, mat3x2<f32>>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<private_, f32>(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowVectorElementPointer});
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Ptr_AddressSpace) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<storage, array<f32, 2>, read>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<uniform, f32, read>(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:34 error: access: result of access chain is type 'ptr<storage, f32, read>' but instruction type is 'ptr<uniform, f32, read>'
    %3:ptr<uniform, f32, read> = access %2, 1u
                                 ^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<storage, array<f32, 2>, read>):void {
  $B1: {
    %3:ptr<uniform, f32, read> = access %2, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_Incorrect_Ptr_Access) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.ptr<storage, array<f32, 2>, read>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.ptr<storage, f32, read_write>(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:40 error: access: result of access chain is type 'ptr<storage, f32, read>' but instruction type is 'ptr<storage, f32, read_write>'
    %3:ptr<storage, f32, read_write> = access %2, 1u
                                       ^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func(%2:ptr<storage, array<f32, 2>, read>):void {
  $B1: {
    %3:ptr<storage, f32, read_write> = access %2, 1u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Access_IndexVector) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.vec3<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Access_IndexVector_ViaMatrix) {
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam(ty.mat3x2<f32>());
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ty.f32(), obj, 1_u, 1_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Access_ExtractPointerFromStruct) {
    auto* ptr = ty.ptr<private_, i32>();
    Vector<type::Manager::StructMemberDesc, 1> members{
        type::Manager::StructMemberDesc{mod.symbols.New("a"), ptr},
    };
    auto* str = ty.Struct(mod.symbols.New("MyStruct"), std::move(members));
    auto* f = b.Function("my_func", ty.void_());
    auto* obj = b.FunctionParam("obj", str);
    f->SetParams({obj});

    b.Append(f->Block(), [&] {
        b.Access(ptr, obj, 0_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowPointersInStructures});
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Block_TerminatorInMiddle) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Return(f);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: return: must be the last instruction in the block
    ret
    ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    ret
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, If_EmptyFalse) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->True()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, If_EmptyTrue) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->False()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:7 error: block does not end in a terminator instruction
      $B2: {  # true
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
      }
      $B3: {  # false
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, If_ConditionIsBool) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(1_i);
    if_->True()->Append(b.Return(f));
    if_->False()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:8 error: if: condition type must be 'bool'
    if 1i [t: $B2, f: $B3] {  # if_1
       ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if 1i [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        ret
      }
      $B3: {  # false
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, If_ConditionIsNullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(nullptr);
    if_->True()->Append(b.Return(f));
    if_->False()->Append(b.Return(f));

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:8 error: if: operand is undefined
    if undef [t: $B2, f: $B3] {  # if_1
       ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if undef [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        ret
      }
      $B3: {  # false
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, If_NullResult) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->True()->Append(b.Return(f));
    if_->False()->Append(b.Return(f));

    if_->SetResults(Vector<InstructionResult*, 1>{nullptr});

    f->Block()->Append(if_);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: if: result is undefined
    undef = if true [t: $B2, f: $B3] {  # if_1
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = if true [t: $B2, f: $B3] {  # if_1
      $B2: {  # true
        ret
      }
      $B3: {  # false
        ret
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Loop_OnlyBody) {
    auto* f = b.Function("my_func", ty.void_());

    auto* l = b.Loop();
    l->Body()->Append(b.ExitLoop(l));

    auto sb = b.Append(f->Block());
    sb.Append(l);
    sb.Return(f);

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, Loop_EmptyBody) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(b.Loop());
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:7 error: block does not end in a terminator instruction
      $B2: {  # body
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_RootBlock_NullResult) {
    auto* v = mod.CreateInstruction<ir::Var>(nullptr);
    v->SetInitializer(b.Constant(0_i));
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:2:3 error: var: result is undefined
  undef = var, 0i
  ^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  undef = var, 0i
}

)");
}

TEST_F(IR_ValidatorTest, Var_Function_NullResult) {
    auto* v = mod.CreateInstruction<ir::Var>(nullptr);
    v->SetInitializer(b.Constant(0_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: var: result is undefined
    undef = var, 0i
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = var, 0i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Function_NoResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        v->SetInitializer(b.Constant(1_i));
        v->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:13 error: var: expected exactly 1 results, got 0
    undef = var, 1i
            ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = var, 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Function_NonPtrResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        v->Result(0)->SetType(ty.f32());
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:14 error: var: result type must be a pointer or a reference
    %2:f32 = var
             ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Function_UnexpectedInputAttachmentIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        v->SetInputAttachmentIndex(0);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:41 error: var: '@input_attachment_index' is not valid for non-handle var
    %2:ptr<function, f32, read_write> = var @input_attachment_index(0)
                                        ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var @input_attachment_index(0)
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Private_UnexpectedInputAttachmentIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<private_, f32>();

        v->SetInputAttachmentIndex(0);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:40 error: var: '@input_attachment_index' is not valid for non-handle var
    %2:ptr<private, f32, read_write> = var @input_attachment_index(0)
                                       ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<private, f32, read_write> = var @input_attachment_index(0)
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_PushConstant_UnexpectedInputAttachmentIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<push_constant, f32>();
        v->SetInputAttachmentIndex(0);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:40 error: var: '@input_attachment_index' is not valid for non-handle var
    %2:ptr<push_constant, f32, read> = var @input_attachment_index(0)
                                       ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<push_constant, f32, read> = var @input_attachment_index(0)
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Storage_UnexpectedInputAttachmentIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<storage, f32>();
        v->SetBindingPoint(0, 0);
        v->SetInputAttachmentIndex(0);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:40 error: var: '@input_attachment_index' is not valid for non-handle var
    %2:ptr<storage, f32, read_write> = var @binding_point(0, 0) @input_attachment_index(0)
                                       ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<storage, f32, read_write> = var @binding_point(0, 0) @input_attachment_index(0)
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Uniform_UnexpectedInputAttachmentIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<uniform, f32>();
        v->SetBindingPoint(0, 0);
        v->SetInputAttachmentIndex(0);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:34 error: var: '@input_attachment_index' is not valid for non-handle var
    %2:ptr<uniform, f32, read> = var @binding_point(0, 0) @input_attachment_index(0)
                                 ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<uniform, f32, read> = var @binding_point(0, 0) @input_attachment_index(0)
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Workgroup_UnexpectedInputAttachmentIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<workgroup, f32>();
        v->SetInputAttachmentIndex(0);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:42 error: var: '@input_attachment_index' is not valid for non-handle var
    %2:ptr<workgroup, f32, read_write> = var @input_attachment_index(0)
                                         ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<workgroup, f32, read_write> = var @input_attachment_index(0)
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Init_WrongType) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        v->SetInitializer(b.Constant(1_i));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:41 error: var: initializer type 'i32' does not match store type 'f32'
    %2:ptr<function, f32, read_write> = var, 1i
                                        ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var, 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Init_NullType) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* i = b.Var<function, f32>("i");
        i->SetInitializer(b.Constant(0_f));
        auto* load = b.Load(i);
        auto* load_ret = load->Result(0);
        auto* j = b.Var<function, f32>("j");
        j->SetInitializer(load_ret);
        load_ret->SetType(nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:5 error: load: result type is undefined
    %3:undef = load %i
    ^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:5:46 error: var: operand type is undefined
    %j:ptr<function, f32, read_write> = var, %3
                                             ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %i:ptr<function, f32, read_write> = var, 0.0f
    %3:undef = load %i
    %j:ptr<function, f32, read_write> = var, %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_HandleMissingBindingPoint) {
    auto* v = b.Var(ty.ptr<handle, i32>());
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:31 error: var: resource variable missing binding points
  %1:ptr<handle, i32, read> = var
                              ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<handle, i32, read> = var
}

)");
}

TEST_F(IR_ValidatorTest, Var_StorageMissingBindingPoint) {
    auto* v = b.Var(ty.ptr<storage, i32>());
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:38 error: var: resource variable missing binding points
  %1:ptr<storage, i32, read_write> = var
                                     ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<storage, i32, read_write> = var
}

)");
}

TEST_F(IR_ValidatorTest, Var_UniformMissingBindingPoint) {
    auto* v = b.Var(ty.ptr<uniform, i32>());
    mod.root_block->Append(v);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:32 error: var: resource variable missing binding points
  %1:ptr<uniform, i32, read> = var
                               ^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:ptr<uniform, i32, read> = var
}

)");
}

TEST_F(IR_ValidatorTest, Var_Basic_BothLocationAndBuiltin) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* v = b.Var<function, f32>();
        IOAttributes attr;
        attr.builtin = BuiltinValue::kPosition;
        attr.location = 0;
        v->SetAttributes(attr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:41 error: var: a builtin and location cannot be both declared for a var
    %2:ptr<function, f32, read_write> = var @location(0) @builtin(position)
                                        ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var @location(0) @builtin(position)
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Var_Struct_BothLocationAndBuiltin) {
    IOAttributes attr;
    attr.builtin = BuiltinValue::kPosition;
    attr.location = 0;

    auto* str_ty =
        ty.Struct(mod.symbols.New("MyStruct"), {
                                                   {mod.symbols.New("a"), ty.f32(), attr},
                                               });
    auto* v = b.Var(ty.ptr(storage, str_ty, read_write));
    v->SetBindingPoint(0, 0);
    mod.root_block->Append(v);

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] { b.Return(f); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:6:43 error: var: a builtin and location cannot be both declared for a struct member
  %1:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
                                          ^^^

:5:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
MyStruct = struct @align(4) {
  a:f32 @offset(0), @location(0), @builtin(position)
}

$B1: {  # root
  %1:ptr<storage, MyStruct, read_write> = var @binding_point(0, 0)
}

%my_func = func():void {
  $B2: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_NullResult) {
    auto* v = mod.CreateInstruction<ir::Let>(nullptr, b.Constant(1_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: let: result is undefined
    undef = let 1i
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = let 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_EmptyResults) {
    auto* v = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.i32()), b.Constant(1_i));
    v->ClearResults();

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:13 error: let: expected exactly 1 results, got 0
    undef = let 1i
            ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = let 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_NullValue) {
    auto* v = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.f32()), nullptr);

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:18 error: let: operand is undefined
    %2:f32 = let undef
                 ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = let undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_EmptyValue) {
    auto* v = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.i32()), b.Constant(1_i));
    v->ClearOperands();

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:14 error: let: expected exactly 1 operands, got 0
    %2:i32 = let
             ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = let
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Let_WrongType) {
    auto* v = mod.CreateInstruction<ir::Let>(b.InstructionResult(ty.f32()), b.Constant(1_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(v);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:14 error: let: result type 'f32' does not match value type 'i32'
    %2:f32 = let 1i
             ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = let 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Instruction_AppendedDead) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    auto* ret = sb.Return(f);

    v->Destroy();
    v->InsertBefore(ret);

    auto addr = tint::ToString(v);
    auto arrows = std::string(addr.length(), '^');

    std::string expected = R"(:3:5 error: var: destroyed instruction found in instruction list
    <destroyed tint::core::ir::Var $ADDRESS>
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^$ARROWS^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    <destroyed tint::core::ir::Var $ADDRESS>
    ret
  }
}
)";

    expected = tint::ReplaceAll(expected, "$ADDRESS", addr);
    expected = tint::ReplaceAll(expected, "$ARROWS", arrows);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), expected);
}

TEST_F(IR_ValidatorTest, Instruction_NullInstruction) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    v->Result(0)->SetInstruction(nullptr);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: var: result instruction is undefined
    %2:ptr<function, f32, read_write> = var
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Instruction_DeadOperand) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    auto* result = sb.InstructionResult(ty.f32());
    result->Destroy();
    v->SetInitializer(result);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:46 error: var: operand is not alive
    %2:ptr<function, f32, read_write> = var, %3
                                             ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var, %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Instruction_OperandUsageRemoved) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    sb.Return(f);

    auto* result = sb.InstructionResult(ty.f32());
    v->SetInitializer(result);
    result->RemoveUsage({v, 0u});

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:46 error: var: operand missing usage
    %2:ptr<function, f32, read_write> = var, %3
                                             ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var, %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Instruction_OrphanedInstruction) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* v = sb.Var(ty.ptr<function, f32>());
    auto* load = sb.Load(v);
    sb.Return(f);

    load->Remove();

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(error: load: orphaned instruction: load
note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Binary_LHS_Nullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Add(ty.i32(), nullptr, sb.Constant(2_i));
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:18 error: binary: operand is undefined
    %2:i32 = add undef, 2i
                 ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = add undef, 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Binary_RHS_Nullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Add(ty.i32(), sb.Constant(2_i), nullptr);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:22 error: binary: operand is undefined
    %2:i32 = add 2i, undef
                     ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = add 2i, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Binary_Result_Nullptr) {
    auto* bin = mod.CreateInstruction<ir::CoreBinary>(nullptr, BinaryOp::kAdd, b.Constant(3_i),
                                                      b.Constant(2_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: binary: result is undefined
    undef = add 3i, 2i
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = add 3i, 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Binary_MissingOperands) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* add = sb.Add(ty.i32(), sb.Constant(1_i), sb.Constant(2_i));
    add->ClearOperands();
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: binary: expected at least 2 operands, got 0
    %2:i32 = add
    ^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = add
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Binary_MissingResult) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* add = sb.Add(ty.i32(), sb.Constant(1_i), sb.Constant(2_i));
    add->ClearResults();
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: binary: expected exactly 1 results, got 0
    undef = add 1i, 2i
    ^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = add 1i, 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Unary_Value_Nullptr) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Negation(ty.i32(), nullptr);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:23 error: unary: operand is undefined
    %2:i32 = negation undef
                      ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = negation undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Unary_Result_Nullptr) {
    auto* bin = mod.CreateInstruction<ir::CoreUnary>(nullptr, UnaryOp::kNegation, b.Constant(2_i));

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: unary: result is undefined
    undef = negation 2i
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = negation 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Unary_ResultTypeNotMatchValueType) {
    auto* bin = b.Complement(ty.f32(), 2_i);

    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    sb.Append(bin);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:5 error: unary: result value type 'f32' does not match complement result type 'i32'
    %2:f32 = complement 2i
    ^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = complement 2i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Unary_MissingOperands) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* u = b.Negation(ty.f32(), 2_f);
    u->ClearOperands();
    sb.Append(u);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: unary: expected at least 1 operands, got 0
    %2:f32 = negation
    ^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = negation
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Unary_MissingResults) {
    auto* f = b.Function("my_func", ty.void_());

    auto sb = b.Append(f->Block());
    auto* u = b.Negation(ty.f32(), 2_f);
    u->ClearResults();
    sb.Append(u);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: unary: expected exactly 1 results, got 0
    undef = negation 2.0f
    ^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    undef = negation 2.0f
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, ExitIf_NullIf) {
    auto* if_ = b.If(true);
    if_->True()->Append(mod.CreateInstruction<ExitIf>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:5:9 error: exit_if: has no parent control instruction
        exit_if  # undef
        ^^^^^^^

:4:7 note: in block
      $B2: {  # true
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        exit_if  # undef
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_LessOperandsThenIfParams) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: exit_if: provides 1 value but 'if' expects 2 values
        exit_if 1i  # if_1
        ^^^^^^^^^^

:4:7 note: in block
      $B2: {  # true
      ^^^

:3:5 note: 'if' declared here
    %2:i32, %3:f32 = if true [t: $B2] {  # if_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = if true [t: $B2] {  # if_1
      $B2: {  # true
        exit_if 1i  # if_1
      }
      # implicit false block: exit_if undef, undef
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_MoreOperandsThenIfParams) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i, 2_f, 3_i));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: exit_if: provides 3 values but 'if' expects 2 values
        exit_if 1i, 2.0f, 3i  # if_1
        ^^^^^^^^^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # true
      ^^^

:3:5 note: 'if' declared here
    %2:i32, %3:f32 = if true [t: $B2] {  # if_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = if true [t: $B2] {  # if_1
      $B2: {  # true
        exit_if 1i, 2.0f, 3i  # if_1
      }
      # implicit false block: exit_if undef, undef
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_WithResult) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i, 2_f));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, ExitIf_IncorrectResultType) {
    auto* if_ = b.If(true);
    if_->True()->Append(b.ExitIf(if_, 1_i, 2_i));

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    if_->SetResults(Vector{r1, r2});

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:21 error: exit_if: operand with type 'i32' does not match 'if' target type 'f32'
        exit_if 1i, 2i  # if_1
                    ^^

:4:7 note: in block
      $B2: {  # true
      ^^^

:3:13 note: %3 declared here
    %2:i32, %3:f32 = if true [t: $B2] {  # if_1
            ^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = if true [t: $B2] {  # if_1
      $B2: {  # true
        exit_if 1i, 2i  # if_1
      }
      # implicit false block: exit_if undef, undef
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_NotInParentIf) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_ = b.If(true);
    if_->True()->Append(b.Return(f));

    auto sb = b.Append(f->Block());
    sb.Append(if_);
    sb.ExitIf(if_);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:5 error: exit_if: found outside all control instructions
    exit_if  # if_1
    ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        ret
      }
    }
    exit_if  # if_1
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_InvalidJumpsOverIf) {
    auto* f = b.Function("my_func", ty.void_());

    auto* if_inner = b.If(true);

    auto* if_outer = b.If(true);
    b.Append(if_outer->True(), [&] {
        b.Append(if_inner);
        b.ExitIf(if_outer);
    });

    b.Append(if_inner->True(), [&] { b.ExitIf(if_outer); });

    b.Append(f->Block(), [&] {
        b.Append(if_outer);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_if: if target jumps over other control instructions
            exit_if  # if_1
            ^^^^^^^

:6:11 note: in block
          $B3: {  # true
          ^^^

:5:9 note: first control instruction jumped
        if true [t: $B3] {  # if_2
        ^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        if true [t: $B3] {  # if_2
          $B3: {  # true
            exit_if  # if_1
          }
        }
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_InvalidJumpOverSwitch) {
    auto* f = b.Function("my_func", ty.void_());

    auto* switch_inner = b.Switch(1_i);

    auto* if_outer = b.If(true);
    b.Append(if_outer->True(), [&] {
        b.Append(switch_inner);
        b.ExitIf(if_outer);
    });

    auto* c = b.Case(switch_inner, {b.Constant(1_i), nullptr});
    b.Append(c, [&] { b.ExitIf(if_outer); });

    b.Append(f->Block(), [&] {
        b.Append(if_outer);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_if: if target jumps over other control instructions
            exit_if  # if_1
            ^^^^^^^

:6:11 note: in block
          $B3: {  # case
          ^^^

:5:9 note: first control instruction jumped
        switch 1i [c: (1i default, $B3)] {  # switch_1
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        switch 1i [c: (1i default, $B3)] {  # switch_1
          $B3: {  # case
            exit_if  # if_1
          }
        }
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitIf_InvalidJumpOverLoop) {
    auto* f = b.Function("my_func", ty.void_());

    auto* loop = b.Loop();

    auto* if_outer = b.If(true);
    b.Append(if_outer->True(), [&] {
        b.Append(loop);
        b.ExitIf(if_outer);
    });

    b.Append(loop->Body(), [&] { b.ExitIf(if_outer); });

    b.Append(f->Block(), [&] {
        b.Append(if_outer);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_if: if target jumps over other control instructions
            exit_if  # if_1
            ^^^^^^^

:6:11 note: in block
          $B3: {  # body
          ^^^

:5:9 note: first control instruction jumped
        loop [b: $B3] {  # loop_1
        ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    if true [t: $B2] {  # if_1
      $B2: {  # true
        loop [b: $B3] {  # loop_1
          $B3: {  # body
            exit_if  # if_1
          }
        }
        exit_if  # if_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch) {
    auto* switch_ = b.Switch(1_i);

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, ExitSwitch_NullSwitch) {
    auto* switch_ = b.Switch(1_i);

    auto* def = b.DefaultCase(switch_);
    def->Append(mod.CreateInstruction<ExitSwitch>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: exit_switch: has no parent control instruction
        exit_switch  # undef
        ^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # case
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    switch 1i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch  # undef
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch_LessOperandsThenSwitchParams) {
    auto* switch_ = b.Switch(1_i);

    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(Vector{r1, r2});

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_, 1_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: exit_switch: provides 1 value but 'switch' expects 2 values
        exit_switch 1i  # switch_1
        ^^^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # case
      ^^^

:3:5 note: 'switch' declared here
    %2:i32, %3:f32 = switch 1i [c: (default, $B2)] {  # switch_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = switch 1i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch 1i  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch_MoreOperandsThenSwitchParams) {
    auto* switch_ = b.Switch(1_i);
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(Vector{r1, r2});

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_, 1_i, 2_f, 3_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: exit_switch: provides 3 values but 'switch' expects 2 values
        exit_switch 1i, 2.0f, 3i  # switch_1
        ^^^^^^^^^^^^^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # case
      ^^^

:3:5 note: 'switch' declared here
    %2:i32, %3:f32 = switch 1i [c: (default, $B2)] {  # switch_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = switch 1i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch 1i, 2.0f, 3i  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch_WithResult) {
    auto* switch_ = b.Switch(1_i);
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(Vector{r1, r2});

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_, 1_i, 2_f));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, ExitSwitch_IncorrectResultType) {
    auto* switch_ = b.Switch(1_i);
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    switch_->SetResults(Vector{r1, r2});

    auto* def = b.DefaultCase(switch_);
    def->Append(b.ExitSwitch(switch_, 1_i, 2_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:25 error: exit_switch: operand with type 'i32' does not match 'switch' target type 'f32'
        exit_switch 1i, 2i  # switch_1
                        ^^

:4:7 note: in block
      $B2: {  # case
      ^^^

:3:13 note: %3 declared here
    %2:i32, %3:f32 = switch 1i [c: (default, $B2)] {  # switch_1
            ^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = switch 1i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch 1i, 2i  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch_NotInParentSwitch) {
    auto* switch_ = b.Switch(1_i);

    auto* f = b.Function("my_func", ty.void_());

    auto* def = b.DefaultCase(switch_);
    def->Append(b.Return(f));

    auto sb = b.Append(f->Block());
    sb.Append(switch_);

    auto* if_ = sb.Append(b.If(true));
    b.Append(if_->True(), [&] { b.ExitSwitch(switch_); });
    sb.Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:10:9 error: exit_switch: switch not found in parent control instructions
        exit_switch  # switch_1
        ^^^^^^^^^^^

:9:7 note: in block
      $B3: {  # true
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    switch 1i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        ret
      }
    }
    if true [t: $B3] {  # if_1
      $B3: {  # true
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch_JumpsOverIfs) {
    // switch(true) {
    //   default: {
    //     if (true) {
    //      if (false) {
    //         break;
    //       }
    //     }
    //     break;
    //  }
    auto* switch_ = b.Switch(1_i);

    auto* f = b.Function("my_func", ty.void_());

    auto* def = b.DefaultCase(switch_);
    b.Append(def, [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            auto* inner_if_ = b.If(false);
            b.Append(inner_if_->True(), [&] { b.ExitSwitch(switch_); });
            b.Return(f);
        });
        b.ExitSwitch(switch_);
    });

    auto sb = b.Append(f->Block());
    sb.Append(switch_);
    sb.Return(f);

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, ExitSwitch_InvalidJumpOverSwitch) {
    auto* switch_ = b.Switch(1_i);

    auto* def = b.DefaultCase(switch_);
    b.Append(def, [&] {
        auto* inner = b.Switch(0_i);
        b.ExitSwitch(switch_);

        auto* inner_def = b.DefaultCase(inner);
        b.Append(inner_def, [&] { b.ExitSwitch(switch_); });
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(switch_);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_switch: switch target jumps over other control instructions
            exit_switch  # switch_1
            ^^^^^^^^^^^

:6:11 note: in block
          $B3: {  # case
          ^^^

:5:9 note: first control instruction jumped
        switch 0i [c: (default, $B3)] {  # switch_2
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    switch 1i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        switch 0i [c: (default, $B3)] {  # switch_2
          $B3: {  # case
            exit_switch  # switch_1
          }
        }
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitSwitch_InvalidJumpOverLoop) {
    auto* switch_ = b.Switch(1_i);

    auto* def = b.DefaultCase(switch_);
    b.Append(def, [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitSwitch(switch_); });
        b.ExitSwitch(switch_);
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(switch_);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_switch: switch target jumps over other control instructions
            exit_switch  # switch_1
            ^^^^^^^^^^^

:6:11 note: in block
          $B3: {  # body
          ^^^

:5:9 note: first control instruction jumped
        loop [b: $B3] {  # loop_1
        ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    switch 1i [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        loop [b: $B3] {  # loop_1
          $B3: {  # body
            exit_switch  # switch_1
          }
        }
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Continue_OutsideOfLoop) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Continue(loop);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:5 error: continue: called outside of associated loop
    continue  # -> $B3
    ^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        exit_loop  # loop_1
      }
    }
    continue  # -> $B3
  }
}
)");
}

TEST_F(IR_ValidatorTest, Continue_InLoopInit) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] { b.Continue(loop); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: continue: must only be called from loop body
        continue  # -> $B4
        ^^^^^^^^

:4:7 note: in block
      $B2: {  # initializer
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        continue  # -> $B4
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Continue_InLoopBody) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, Continue_InLoopContinuing) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] { b.Continue(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:9 error: continue: must only be called from loop body
        continue  # -> $B3
        ^^^^^^^^

:7:7 note: in block
      $B3: {  # continuing
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        exit_loop  # loop_1
      }
      $B3: {  # continuing
        continue  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Continue_UnexpectedValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop, 1_i, 2_f); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: continue: provides 2 values but 'loop' block $B3 expects 0 values
        continue 1i, 2.0f  # -> $B3
        ^^^^^^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # body
      ^^^

:7:7 note: 'loop' block $B3 declared here
      $B3: {  # continuing
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue 1i, 2.0f  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Continue_MissingValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams({b.BlockParam<i32>(), b.BlockParam<i32>()});
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: continue: provides 0 values but 'loop' block $B3 expects 2 values
        continue  # -> $B3
        ^^^^^^^^

:4:7 note: in block
      $B2: {  # body
      ^^^

:7:7 note: 'loop' block $B3 declared here
      $B3 (%2:i32, %3:i32): {  # continuing
      ^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3 (%2:i32, %3:i32): {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Continue_MismatchedTypes) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Body(), [&] { b.Continue(loop, 1_i, 2_i, 3_f, false); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:22 error: continue: operand with type 'i32' does not match 'loop' block $B3 target type 'f32'
        continue 1i, 2i, 3.0f, false  # -> $B3
                     ^^

:4:7 note: in block
      $B2: {  # body
      ^^^

:7:20 note: %3 declared here
      $B3 (%2:i32, %3:f32, %4:u32, %5:bool): {  # continuing
                   ^^

:5:26 error: continue: operand with type 'f32' does not match 'loop' block $B3 target type 'u32'
        continue 1i, 2i, 3.0f, false  # -> $B3
                         ^^^^

:4:7 note: in block
      $B2: {  # body
      ^^^

:7:28 note: %4 declared here
      $B3 (%2:i32, %3:f32, %4:u32, %5:bool): {  # continuing
                           ^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue 1i, 2i, 3.0f, false  # -> $B3
      }
      $B3 (%2:i32, %3:f32, %4:u32, %5:bool): {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Continue_MatchedTypes) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Body(), [&] { b.Continue(loop, 1_i, 2_f, 3_u, false); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, NextIteration_OutsideOfLoop) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.NextIteration(loop);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:5 error: next_iteration: called outside of associated loop
    next_iteration  # -> $B2
    ^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        exit_loop  # loop_1
      }
    }
    next_iteration  # -> $B2
  }
}
)");
}

TEST_F(IR_ValidatorTest, NextIteration_InLoopInit) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, NextIteration_InLoopBody) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.NextIteration(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: next_iteration: must only be called from loop initializer or continuing
        next_iteration  # -> $B2
        ^^^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # body
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2: {  # body
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, NextIteration_InLoopContinuing) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Append(loop->Continuing(), [&] { b.NextIteration(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, NextIteration_UnexpectedValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop, 1_i, 2_f); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: next_iteration: provides 2 values but 'loop' block $B3 expects 0 values
        next_iteration 1i, 2.0f  # -> $B3
        ^^^^^^^^^^^^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # initializer
      ^^^

:7:7 note: 'loop' block $B3 declared here
      $B3: {  # body
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        next_iteration 1i, 2.0f  # -> $B3
      }
      $B3: {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, NextIteration_MissingValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams({b.BlockParam<i32>(), b.BlockParam<i32>()});
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: next_iteration: provides 0 values but 'loop' block $B3 expects 2 values
        next_iteration  # -> $B3
        ^^^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # initializer
      ^^^

:7:7 note: 'loop' block $B3 declared here
      $B3 (%2:i32, %3:i32): {  # body
      ^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        next_iteration  # -> $B3
      }
      $B3 (%2:i32, %3:i32): {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, NextIteration_MismatchedTypes) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop, 1_i, 2_i, 3_f, false); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:28 error: next_iteration: operand with type 'i32' does not match 'loop' block $B3 target type 'f32'
        next_iteration 1i, 2i, 3.0f, false  # -> $B3
                           ^^

:4:7 note: in block
      $B2: {  # initializer
      ^^^

:7:20 note: %3 declared here
      $B3 (%2:i32, %3:f32, %4:u32, %5:bool): {  # body
                   ^^

:5:32 error: next_iteration: operand with type 'f32' does not match 'loop' block $B3 target type 'u32'
        next_iteration 1i, 2i, 3.0f, false  # -> $B3
                               ^^^^

:4:7 note: in block
      $B2: {  # initializer
      ^^^

:7:28 note: %4 declared here
      $B3 (%2:i32, %3:f32, %4:u32, %5:bool): {  # body
                           ^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [i: $B2, b: $B3] {  # loop_1
      $B2: {  # initializer
        next_iteration 1i, 2i, 3.0f, false  # -> $B3
      }
      $B3 (%2:i32, %3:f32, %4:u32, %5:bool): {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, NextIteration_MatchedTypes) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop, 1_i, 2_f, 3_u, false); });
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, LoopBodyParamsWithoutInitializer) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams({b.BlockParam<i32>(), b.BlockParam<i32>()});
        b.Append(loop->Body(), [&] { b.ExitLoop(loop); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: loop: loop with body block parameters must have an initializer
    loop [b: $B2] {  # loop_1
    ^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2] {  # loop_1
      $B2 (%2:i32, %3:i32): {  # body
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ContinuingUseValueBeforeContinue) {
    auto* f = b.Function("my_func", ty.void_());
    auto* value = b.Let("value", 1_i);
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            b.Append(value);
            b.Append(b.If(true)->True(), [&] { b.Continue(loop); });
            b.ExitLoop(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Let("use", value);
            b.NextIteration(loop);
        });
        b.Return(f);
    });

    ASSERT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, ContinuingUseValueAfterContinue) {
    auto* f = b.Function("my_func", ty.void_());
    auto* value = b.Let("value", 1_i);
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] {
            b.Append(b.If(true)->True(), [&] { b.Continue(loop); });
            b.Append(value);
            b.ExitLoop(loop);
        });
        b.Append(loop->Continuing(), [&] {
            b.Let("use", value);
            b.NextIteration(loop);
        });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:14:24 error: let: %value cannot be used in continuing block as it is declared after the first 'continue' in the loop's body
        %use:i32 = let %value
                       ^^^^^^

:4:7 note: in block
      $B2: {  # body
      ^^^

:10:9 note: %value declared here
        %value:i32 = let 1i
        ^^^^^^^^^^

:7:13 note: loop body's first 'continue'
            continue  # -> $B3
            ^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        if true [t: $B4] {  # if_1
          $B4: {  # true
            continue  # -> $B3
          }
        }
        %value:i32 = let 1i
        exit_loop  # loop_1
      }
      $B3: {  # continuing
        %use:i32 = let %value
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, BreakIf_NextIterUnexpectedValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true, b.Values(1_i, 2_i), Empty); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:9 error: break_if: provides 2 values but 'loop' block $B2 expects 0 values
        break_if true next_iteration: [ 1i, 2i ]  # -> [t: exit_loop loop_1, f: $B2]
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:7:7 note: in block
      $B3: {  # continuing
      ^^^

:4:7 note: 'loop' block $B2 declared here
      $B2: {  # body
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true next_iteration: [ 1i, 2i ]  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, BreakIf_NextIterMissingValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams({b.BlockParam<i32>(), b.BlockParam<i32>()});
        b.Append(loop->Initializer(), [&] { b.NextIteration(loop, nullptr, nullptr); });
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true, Empty, Empty); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:11:9 error: break_if: provides 0 values but 'loop' block $B3 expects 2 values
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
        ^^^^^^^^^^^^^

:10:7 note: in block
      $B4: {  # continuing
      ^^^

:7:7 note: 'loop' block $B3 declared here
      $B3 (%2:i32, %3:i32): {  # body
      ^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration undef, undef  # -> $B3
      }
      $B3 (%2:i32, %3:i32): {  # body
        continue  # -> $B4
      }
      $B4: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, BreakIf_NextIterMismatchedTypes) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Initializer(),
                 [&] { b.NextIteration(loop, nullptr, nullptr, nullptr, nullptr); });
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(),
                 [&] { b.BreakIf(loop, true, b.Values(1_i, 2_i, 3_f, false), Empty); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:11:45 error: break_if: operand with type 'i32' does not match 'loop' block $B3 target type 'f32'
        break_if true next_iteration: [ 1i, 2i, 3.0f, false ]  # -> [t: exit_loop loop_1, f: $B3]
                                            ^^

:10:7 note: in block
      $B4: {  # continuing
      ^^^

:7:20 note: %3 declared here
      $B3 (%2:i32, %3:f32, %4:u32, %5:bool): {  # body
                   ^^

:11:49 error: break_if: operand with type 'f32' does not match 'loop' block $B3 target type 'u32'
        break_if true next_iteration: [ 1i, 2i, 3.0f, false ]  # -> [t: exit_loop loop_1, f: $B3]
                                                ^^^^

:10:7 note: in block
      $B4: {  # continuing
      ^^^

:7:28 note: %4 declared here
      $B3 (%2:i32, %3:f32, %4:u32, %5:bool): {  # body
                           ^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        next_iteration undef, undef, undef, undef  # -> $B3
      }
      $B3 (%2:i32, %3:f32, %4:u32, %5:bool): {  # body
        continue  # -> $B4
      }
      $B4: {  # continuing
        break_if true next_iteration: [ 1i, 2i, 3.0f, false ]  # -> [t: exit_loop loop_1, f: $B3]
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, BreakIf_NextIterMatchedTypes) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->Body()->SetParams(
            {b.BlockParam<i32>(), b.BlockParam<f32>(), b.BlockParam<u32>(), b.BlockParam<bool>()});
        b.Append(loop->Initializer(),
                 [&] { b.NextIteration(loop, nullptr, nullptr, nullptr, nullptr); });
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(),
                 [&] { b.BreakIf(loop, true, b.Values(1_i, 2_f, 3_u, false), Empty); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, BreakIf_ExitUnexpectedValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true, Empty, b.Values(1_i, 2_i)); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:9 error: break_if: provides 2 values but 'loop' expects 0 values
        break_if true exit_loop: [ 1i, 2i ]  # -> [t: exit_loop loop_1, f: $B2]
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:7:7 note: in block
      $B3: {  # continuing
      ^^^

:3:5 note: 'loop' declared here
    loop [b: $B2, c: $B3] {  # loop_1
    ^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true exit_loop: [ 1i, 2i ]  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, BreakIf_ExitMissingValues) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->SetResults(b.InstructionResult<i32>(), b.InstructionResult<i32>());
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(), [&] { b.BreakIf(loop, true, Empty, Empty); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:9 error: break_if: provides 0 values but 'loop' expects 2 values
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
        ^^^^^^^^^^^^^

:7:7 note: in block
      $B3: {  # continuing
      ^^^

:3:5 note: 'loop' declared here
    %2:i32, %3:i32 = loop [b: $B2, c: $B3] {  # loop_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:i32 = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, BreakIf_ExitMismatchedTypes) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->SetResults(b.InstructionResult<i32>(), b.InstructionResult<f32>(),
                         b.InstructionResult<u32>(), b.InstructionResult<bool>());
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(),
                 [&] { b.BreakIf(loop, true, Empty, b.Values(1_i, 2_i, 3_f, false)); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:8:40 error: break_if: operand with type 'i32' does not match 'loop' target type 'f32'
        break_if true exit_loop: [ 1i, 2i, 3.0f, false ]  # -> [t: exit_loop loop_1, f: $B2]
                                       ^^

:7:7 note: in block
      $B3: {  # continuing
      ^^^

:3:13 note: %3 declared here
    %2:i32, %3:f32, %4:u32, %5:bool = loop [b: $B2, c: $B3] {  # loop_1
            ^^^^^^

:8:44 error: break_if: operand with type 'f32' does not match 'loop' target type 'u32'
        break_if true exit_loop: [ 1i, 2i, 3.0f, false ]  # -> [t: exit_loop loop_1, f: $B2]
                                           ^^^^

:7:7 note: in block
      $B3: {  # continuing
      ^^^

:3:21 note: %4 declared here
    %2:i32, %3:f32, %4:u32, %5:bool = loop [b: $B2, c: $B3] {  # loop_1
                    ^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32, %4:u32, %5:bool = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        break_if true exit_loop: [ 1i, 2i, 3.0f, false ]  # -> [t: exit_loop loop_1, f: $B2]
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, BreakIf_ExitMatchedTypes) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* loop = b.Loop();
        loop->SetResults(b.InstructionResult<i32>(), b.InstructionResult<f32>(),
                         b.InstructionResult<u32>(), b.InstructionResult<bool>());
        b.Append(loop->Body(), [&] { b.Continue(loop); });
        b.Append(loop->Continuing(),
                 [&] { b.BreakIf(loop, true, Empty, b.Values(1_i, 2_f, 3_u, false)); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, ExitLoop) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, ExitLoop_NullLoop) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(mod.CreateInstruction<ExitLoop>(nullptr));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: exit_loop: has no parent control instruction
        exit_loop  # undef
        ^^^^^^^^^

:4:7 note: in block
      $B2: {  # body
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        exit_loop  # undef
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_LessOperandsThenLoopParams) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: exit_loop: provides 1 value but 'loop' expects 2 values
        exit_loop 1i  # loop_1
        ^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # body
      ^^^

:3:5 note: 'loop' declared here
    %2:i32, %3:f32 = loop [b: $B2, c: $B3] {  # loop_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        exit_loop 1i  # loop_1
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_MoreOperandsThenLoopParams) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i, 2_f, 3_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: exit_loop: provides 3 values but 'loop' expects 2 values
        exit_loop 1i, 2.0f, 3i  # loop_1
        ^^^^^^^^^^^^^^^^^^^^^^

:4:7 note: in block
      $B2: {  # body
      ^^^

:3:5 note: 'loop' declared here
    %2:i32, %3:f32 = loop [b: $B2, c: $B3] {  # loop_1
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        exit_loop 1i, 2.0f, 3i  # loop_1
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_WithResult) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i, 2_f));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, ExitLoop_IncorrectResultType) {
    auto* loop = b.Loop();
    auto* r1 = b.InstructionResult(ty.i32());
    auto* r2 = b.InstructionResult(ty.f32());
    loop->SetResults(Vector{r1, r2});

    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.ExitLoop(loop, 1_i, 2_i));

    auto* f = b.Function("my_func", ty.void_());
    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:5:23 error: exit_loop: operand with type 'i32' does not match 'loop' target type 'f32'
        exit_loop 1i, 2i  # loop_1
                      ^^

:4:7 note: in block
      $B2: {  # body
      ^^^

:3:13 note: %3 declared here
    %2:i32, %3:f32 = loop [b: $B2, c: $B3] {  # loop_1
            ^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32, %3:f32 = loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        exit_loop 1i, 2i  # loop_1
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_NotInParentLoop) {
    auto* f = b.Function("my_func", ty.void_());

    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));
    loop->Body()->Append(b.Return(f));

    auto sb = b.Append(f->Block());
    sb.Append(loop);

    auto* if_ = sb.Append(b.If(true));
    b.Append(if_->True(), [&] { b.ExitLoop(loop); });
    sb.Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:13:9 error: exit_loop: loop not found in parent control instructions
        exit_loop  # loop_1
        ^^^^^^^^^

:12:7 note: in block
      $B4: {  # true
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        ret
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    if true [t: $B4] {  # if_1
      $B4: {  # true
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_JumpsOverIfs) {
    // loop {
    //   if (true) {
    //    if (false) {
    //       break;
    //     }
    //   }
    //   break;
    // }
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));

    auto* f = b.Function("my_func", ty.void_());

    b.Append(loop->Body(), [&] {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            auto* inner_if_ = b.If(false);
            b.Append(inner_if_->True(), [&] { b.ExitLoop(loop); });
            b.Return(f);
        });
        b.ExitLoop(loop);
    });

    auto sb = b.Append(f->Block());
    sb.Append(loop);
    sb.Return(f);

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidJumpOverSwitch) {
    auto* loop = b.Loop();
    loop->Continuing()->Append(b.NextIteration(loop));

    b.Append(loop->Body(), [&] {
        auto* inner = b.Switch(1_i);
        b.ExitLoop(loop);

        auto* inner_def = b.DefaultCase(inner);
        b.Append(inner_def, [&] { b.ExitLoop(loop); });
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_loop: loop target jumps over other control instructions
            exit_loop  # loop_1
            ^^^^^^^^^

:6:11 note: in block
          $B4: {  # case
          ^^^

:5:9 note: first control instruction jumped
        switch 1i [c: (default, $B4)] {  # switch_1
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        switch 1i [c: (default, $B4)] {  # switch_1
          $B4: {  # case
            exit_loop  # loop_1
          }
        }
        exit_loop  # loop_1
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidJumpOverLoop) {
    auto* outer_loop = b.Loop();

    outer_loop->Continuing()->Append(b.NextIteration(outer_loop));

    b.Append(outer_loop->Body(), [&] {
        auto* loop = b.Loop();
        b.Append(loop->Body(), [&] { b.ExitLoop(outer_loop); });
        b.ExitLoop(outer_loop);
    });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(outer_loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_loop: loop target jumps over other control instructions
            exit_loop  # loop_1
            ^^^^^^^^^

:6:11 note: in block
          $B4: {  # body
          ^^^

:5:9 note: first control instruction jumped
        loop [b: $B4] {  # loop_2
        ^^^^^^^^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        loop [b: $B4] {  # loop_2
          $B4: {  # body
            exit_loop  # loop_1
          }
        }
        exit_loop  # loop_1
      }
      $B3: {  # continuing
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidInsideContinuing) {
    auto* loop = b.Loop();

    loop->Continuing()->Append(b.ExitLoop(loop));
    loop->Body()->Append(b.Continue(loop));

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:8:9 error: exit_loop: loop exit jumps out of continuing block
        exit_loop  # loop_1
        ^^^^^^^^^

:7:7 note: in block
      $B3: {  # continuing
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        exit_loop  # loop_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidInsideContinuingNested) {
    auto* loop = b.Loop();

    b.Append(loop->Continuing(), [&]() {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&]() { b.ExitLoop(loop); });
        b.NextIteration(loop);
    });

    b.Append(loop->Body(), [&] { b.Continue(loop); });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:10:13 error: exit_loop: loop exit jumps out of continuing block
            exit_loop  # loop_1
            ^^^^^^^^^

:9:11 note: in block
          $B4: {  # true
          ^^^

:7:7 note: in continuing block
      $B3: {  # continuing
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [b: $B2, c: $B3] {  # loop_1
      $B2: {  # body
        continue  # -> $B3
      }
      $B3: {  # continuing
        if true [t: $B4] {  # if_1
          $B4: {  # true
            exit_loop  # loop_1
          }
        }
        next_iteration  # -> $B2
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidInsideInitializer) {
    auto* loop = b.Loop();

    loop->Initializer()->Append(b.ExitLoop(loop));
    loop->Continuing()->Append(b.NextIteration(loop));

    b.Append(loop->Body(), [&] { b.Continue(loop); });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:5:9 error: exit_loop: loop exit not permitted in loop initializer
        exit_loop  # loop_1
        ^^^^^^^^^

:4:7 note: in block
      $B2: {  # initializer
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        exit_loop  # loop_1
      }
      $B3: {  # body
        continue  # -> $B4
      }
      $B4: {  # continuing
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, ExitLoop_InvalidInsideInitializerNested) {
    auto* loop = b.Loop();

    b.Append(loop->Initializer(), [&]() {
        auto* if_ = b.If(true);
        b.Append(if_->True(), [&]() { b.ExitLoop(loop); });
        b.NextIteration(loop);
    });
    loop->Continuing()->Append(b.NextIteration(loop));

    b.Append(loop->Body(), [&] { b.Continue(loop); });

    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(loop);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:7:13 error: exit_loop: loop exit not permitted in loop initializer
            exit_loop  # loop_1
            ^^^^^^^^^

:6:11 note: in block
          $B5: {  # true
          ^^^

:4:7 note: in initializer block
      $B2: {  # initializer
      ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    loop [i: $B2, b: $B3, c: $B4] {  # loop_1
      $B2: {  # initializer
        if true [t: $B5] {  # if_1
          $B5: {  # true
            exit_loop  # loop_1
          }
        }
        next_iteration  # -> $B3
      }
      $B3: {  # body
        continue  # -> $B4
      }
      $B4: {  # continuing
        next_iteration  # -> $B3
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Return) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Return(f);
    });

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, Return_WithValue) {
    auto* f = b.Function("my_func", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Return(f, 42_i);
    });

    EXPECT_EQ(ir::Validate(mod), Success);
}

TEST_F(IR_ValidatorTest, Return_UnexpectedResult) {
    auto* f = b.Function("my_func", ty.i32());
    b.Append(f->Block(), [&] {  //
        auto* r = b.Return(f, 42_i);
        r->SetResults(Vector{b.InstructionResult(ty.i32())});
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: return: expected exactly 0 results, got 1
    ret 42i
    ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():i32 {
  $B1: {
    ret 42i
  }
}
)");
}

TEST_F(IR_ValidatorTest, Return_NotFunction) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        auto* var = b.Var(ty.ptr<function, f32>());
        auto* r = b.Return(nullptr);
        r->SetOperand(0, var->Result(0));
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:4:5 error: return: expected function for first operand
    ret
    ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Return_MissingFunction) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* r = b.Return(f);
        r->ClearOperands();
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: return: expected between 1 and 2 operands, got 0
    ret
    ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Return_UnexpectedValue) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Return(f, 42_i);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: return: unexpected return value
    ret 42i
    ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    ret 42i
  }
}
)");
}

TEST_F(IR_ValidatorTest, Return_MissingValue) {
    auto* f = b.Function("my_func", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:5 error: return: expected return value
    ret
    ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():i32 {
  $B1: {
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Return_WrongValueType) {
    auto* f = b.Function("my_func", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Return(f, 42_f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:5 error: return: return value type 'f32' does not match function return type 'i32'
    ret 42.0f
    ^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():i32 {
  $B1: {
    ret 42.0f
  }
}
)");
}

TEST_F(IR_ValidatorTest, Unreachable_UnexpectedResult) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        auto* u = b.Unreachable();
        u->SetResults(Vector{b.InstructionResult(ty.i32())});
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: unreachable: expected exactly 0 results, got 1
    unreachable
    ^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Unreachable_UnexpectedOperand) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {  //
        auto* u = b.Unreachable();
        u->SetOperands(Vector{b.Value(0_i)});
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: unreachable: expected exactly 0 operands, got 1
    unreachable
    ^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    unreachable
  }
}
)");
}

TEST_F(IR_ValidatorTest, Load_NullFrom) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(mod.CreateInstruction<ir::Load>(b.InstructionResult(ty.i32()), nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:19 error: load: operand is undefined
    %2:i32 = load undef
                  ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = load undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Load_SourceNotMemoryView) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* let = b.Let("l", 1_i);
        b.Append(mod.CreateInstruction<ir::Load>(b.InstructionResult(ty.f32()), let->Result(0)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:19 error: load: load source operand is not a memory view
    %3:f32 = load %l
                  ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %l:i32 = let 1i
    %3:f32 = load %l
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Load_TypeMismatch) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Append(mod.CreateInstruction<ir::Load>(b.InstructionResult(ty.f32()), var->Result(0)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:19 error: load: result type 'f32' does not match source store type 'i32'
    %3:f32 = load %2
                  ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    %3:f32 = load %2
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Load_MissingResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        auto* load = mod.CreateInstruction<ir::Load>(nullptr, var->Result(0));
        load->ClearResults();
        b.Append(load);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:13 error: load: expected exactly 1 results, got 0
    undef = load %2
            ^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    undef = load %2
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_NullTo) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(mod.CreateInstruction<ir::Store>(nullptr, b.Constant(42_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:11 error: store: operand is undefined
    store undef, 42i
          ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    store undef, 42i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_NullFrom) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Append(mod.CreateInstruction<ir::Store>(var->Result(0), nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:4:15 error: store: operand is undefined
    store %2, undef
              ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    store %2, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_NullToAndFrom) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(mod.CreateInstruction<ir::Store>(nullptr, nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:11 error: store: operand is undefined
    store undef, undef
          ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:3:18 error: store: operand is undefined
    store undef, undef
                 ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    store undef, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_NonEmptyResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        auto* store = mod.CreateInstruction<ir::Store>(var->Result(0), b.Constant(42_i));
        store->SetResults(Vector{b.InstructionResult(ty.i32())});
        b.Append(store);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:4:5 error: store: expected exactly 0 results, got 1
    store %2, 42i
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    store %2, 42i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_TargetNotMemoryView) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* let = b.Let("l", 1_i);
        b.Append(mod.CreateInstruction<ir::Store>(let->Result(0), b.Constant(42_u)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:11 error: store: store target operand is not a memory view
    store %l, 42u
          ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %l:i32 = let 1i
    store %l, 42u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_TypeMismatch) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        b.Append(mod.CreateInstruction<ir::Store>(var->Result(0), b.Constant(42_u)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:15 error: store: value type 'u32' does not match store type 'i32'
    store %2, 42u
              ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    store %2, 42u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_NoStoreType) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* result = b.InstructionResult(ty.u32());
        result->SetType(nullptr);
        b.Append(mod.CreateInstruction<ir::Store>(result, b.Constant(42_u)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:11 error: store: operand type is undefined
    store %2, 42u
          ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    store %2, 42u
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Store_NoValueType) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, i32>());
        auto* val = b.Construct(ty.u32(), 42_u);
        val->Result(0)->SetType(nullptr);

        b.Append(mod.CreateInstruction<ir::Store>(var->Result(0), val->Result(0)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:5 error: construct: result type is undefined
    %3:undef = construct 42u
    ^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

:5:15 error: store: operand type is undefined
    store %2, %3
              ^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var
    %3:undef = construct 42u
    store %2, %3
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, LoadVectorElement_NullResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(
            mod.CreateInstruction<ir::LoadVectorElement>(nullptr, var->Result(0), b.Constant(1_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:5 error: load_vector_element: result is undefined
    undef = load_vector_element %2, 1i
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var
    undef = load_vector_element %2, 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, LoadVectorElement_NullFrom) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(mod.CreateInstruction<ir::LoadVectorElement>(b.InstructionResult(ty.f32()),
                                                              nullptr, b.Constant(1_i)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:34 error: load_vector_element: operand is undefined
    %2:f32 = load_vector_element undef, 1i
                                 ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:f32 = load_vector_element undef, 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, LoadVectorElement_NullIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.CreateInstruction<ir::LoadVectorElement>(b.InstructionResult(ty.f32()),
                                                              var->Result(0), nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:4:38 error: load_vector_element: operand is undefined
    %3:f32 = load_vector_element %2, undef
                                     ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var
    %3:f32 = load_vector_element %2, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, LoadVectorElement_MissingResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        auto* load = b.LoadVectorElement(var, b.Constant(1_i));
        load->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:13 error: load_vector_element: expected exactly 1 results, got 0
    undef = load_vector_element %2, 1i
            ^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var
    undef = load_vector_element %2, 1i
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, LoadVectorElement_MissingOperands) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        auto* load = b.LoadVectorElement(var, b.Constant(1_i));
        load->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:14 error: load_vector_element: expected exactly 2 operands, got 0
    %3:f32 = load_vector_element
             ^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var
    %3:f32 = load_vector_element
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, StoreVectorElement_NullTo) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Append(mod.CreateInstruction<ir::StoreVectorElement>(nullptr, b.Constant(1_i),
                                                               b.Constant(2_f)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:3:26 error: store_vector_element: operand is undefined
    store_vector_element undef, 1i, 2.0f
                         ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    store_vector_element undef, 1i, 2.0f
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, StoreVectorElement_NullIndex) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.CreateInstruction<ir::StoreVectorElement>(var->Result(0), nullptr,
                                                               b.Constant(2_f)));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:4:30 error: store_vector_element: operand is undefined
    store_vector_element %2, undef, 2.0f
                             ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var
    store_vector_element %2, undef, 2.0f
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, StoreVectorElement_NullValue) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        b.Append(mod.CreateInstruction<ir::StoreVectorElement>(var->Result(0), b.Constant(1_i),
                                                               nullptr));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(:4:34 error: store_vector_element: operand is undefined
    store_vector_element %2, 1i, undef
                                 ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var
    store_vector_element %2, 1i, undef
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, StoreVectorElement_MissingOperands) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        auto* store = b.StoreVectorElement(var, b.Constant(1_i), b.Constant(2_f));
        store->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:5 error: store_vector_element: expected exactly 3 operands, got 0
    store_vector_element
    ^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var
    store_vector_element
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, StoreVectorElement_UnexpectedResult) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr<function, vec3<f32>>());
        auto* store = b.StoreVectorElement(var, b.Constant(1_i), b.Constant(2_f));
        store->SetResults(Vector{b.InstructionResult(ty.f32())});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:5 error: store_vector_element: expected exactly 0 results, got 1
    store_vector_element %2, 1i, 2.0f
    ^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec3<f32>, read_write> = var
    store_vector_element %2, 1i, 2.0f
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Scoping_UseBeforeDecl) {
    auto* f = b.Function("my_func", ty.void_());

    auto* y = b.Add<i32>(2_i, 3_i);
    auto* x = b.Add<i32>(y, 1_i);

    f->Block()->Append(x);
    f->Block()->Append(y);
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:18 error: binary: %3 is not in scope
    %2:i32 = add %3, 1i
                 ^^

:2:3 note: in block
  $B1: {
  ^^^

:4:5 note: %3 declared here
    %3:i32 = add 2i, 3i
    ^^^^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = add %3, 1i
    %3:i32 = add 2i, 3i
    ret
  }
}
)");
}

template <typename T>
static const type::Type* TypeBuilder(type::Manager& m) {
    return m.Get<T>();
}
template <typename T>
static const type::Type* RefTypeBuilder(type::Manager& m) {
    return m.ref<AddressSpace::kFunction, T>();
}
using TypeBuilderFn = decltype(&TypeBuilder<i32>);

using IR_ValidatorRefTypeTest = IRTestParamHelper<std::tuple</* holds_ref */ bool,
                                                             /* refs_allowed */ bool,
                                                             /* type_builder */ TypeBuilderFn>>;

TEST_P(IR_ValidatorRefTypeTest, Var) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        if (auto* view = type->As<type::MemoryView>()) {
            b.Var(view);
        } else {
            b.Var(ty.ptr<function>(type));
        }

        b.Return(fn);
    });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref || refs_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("3:5 error: var: reference types are not permitted"));
    }
}

TEST_P(IR_ValidatorRefTypeTest, FnParam) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] { b.Return(fn); });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("reference types are not permitted"));
    }
}

TEST_P(IR_ValidatorRefTypeTest, FnRet) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", type);
    b.Append(fn->Block(), [&] { b.Unreachable(); });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("reference types are not permitted"));
    }
}

TEST_P(IR_ValidatorRefTypeTest, BlockParam) {
    bool holds_ref = std::get<0>(GetParam());
    bool refs_allowed = std::get<1>(GetParam());
    auto* type = std::get<2>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams({b.BlockParam(type)});
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop, nullptr);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.NextIteration(loop);
        });
        b.Unreachable();
    });

    Capabilities caps;
    if (refs_allowed) {
        caps.Add(Capability::kAllowRefTypes);
    }
    auto res = ir::Validate(mod, caps);
    if (!holds_ref) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("reference types are not permitted"));
    }
}

INSTANTIATE_TEST_SUITE_P(NonRefTypes,
                         IR_ValidatorRefTypeTest,
                         testing::Combine(/* holds_ref */ testing::Values(false),
                                          /* refs_allowed */ testing::Values(false, true),
                                          /* type_builder */
                                          testing::Values(TypeBuilder<i32>,
                                                          TypeBuilder<bool>,
                                                          TypeBuilder<vec4<f32>>,
                                                          TypeBuilder<array<f32, 3>>)));

INSTANTIATE_TEST_SUITE_P(RefTypes,
                         IR_ValidatorRefTypeTest,
                         testing::Combine(/* holds_ref */ testing::Values(true),
                                          /* refs_allowed */ testing::Values(false, true),
                                          /* type_builder */
                                          testing::Values(RefTypeBuilder<i32>,
                                                          RefTypeBuilder<bool>,
                                                          RefTypeBuilder<vec4<f32>>)));

TEST_F(IR_ValidatorTest, PointerToPointer) {
    auto* type = ty.ptr<function, ptr<function, i32>>();
    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] {  //
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr("nested pointer types are not permitted"));
}

TEST_F(IR_ValidatorTest, ReferenceToReference) {
    auto* type = ty.ref<function>(ty.ref<function, i32>());
    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {  //
        b.Var(type);
        b.Return(fn);
    });

    Capabilities caps;
    caps.Add(Capability::kAllowRefTypes);

    auto res = ir::Validate(mod, caps);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr("nested reference types are not permitted"));
}

TEST_F(IR_ValidatorTest, PointerInStructure_WithoutCapability) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.ptr<private_, i32>()},
                                        });
    mod.root_block->Append(b.Var("my_struct", private_, str_ty));

    auto* fn = b.Function("F", ty.void_());
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_THAT(res.Failure().reason.Str(),
                testing::HasSubstr("nested pointer types are not permitted"));
}

TEST_F(IR_ValidatorTest, PointerInStructure_WithCapability) {
    auto* str_ty =
        ty.Struct(mod.symbols.New("S"), {
                                            {mod.symbols.New("a"), ty.ptr<private_, i32>()},
                                        });

    auto* fn = b.Function("F", ty.void_());
    auto* param = b.FunctionParam("param", str_ty);
    fn->SetParams({param});
    b.Append(fn->Block(), [&] { b.Return(fn); });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllowPointersInStructures});
    EXPECT_EQ(res, Success) << res.Failure();
}

using IR_Validator8BitIntTypeTest = IRTestParamHelper<std::tuple<
    /* int8_allowed */ bool,
    /* type_builder */ TypeBuilderFn>>;

TEST_P(IR_Validator8BitIntTypeTest, Var) {
    bool int8_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Var(ty.ptr<function>(type));
        b.Return(fn);
    });

    Capabilities caps;
    if (int8_allowed) {
        caps.Add(Capability::kAllow8BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int8_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("3:5 error: var: 8-bit integer types are not permitted"));
    }
}

TEST_P(IR_Validator8BitIntTypeTest, FnParam) {
    bool int8_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    fn->SetParams(Vector{b.FunctionParam(type)});
    b.Append(fn->Block(), [&] { b.Return(fn); });

    Capabilities caps;
    if (int8_allowed) {
        caps.Add(Capability::kAllow8BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int8_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("8-bit integer types are not permitted"));
    }
}

TEST_P(IR_Validator8BitIntTypeTest, FnRet) {
    bool int8_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", type);
    b.Append(fn->Block(), [&] { b.Unreachable(); });

    Capabilities caps;
    if (int8_allowed) {
        caps.Add(Capability::kAllow8BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int8_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("8-bit integer types are not permitted"));
    }
}

TEST_P(IR_Validator8BitIntTypeTest, BlockParam) {
    bool int8_allowed = std::get<0>(GetParam());
    auto* type = std::get<1>(GetParam())(ty);

    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        auto* loop = b.Loop();
        loop->Continuing()->SetParams({b.BlockParam(type)});
        b.Append(loop->Body(), [&] {  //
            b.Continue(loop, nullptr);
        });
        b.Append(loop->Continuing(), [&] {  //
            b.NextIteration(loop);
        });
        b.Unreachable();
    });

    Capabilities caps;
    if (int8_allowed) {
        caps.Add(Capability::kAllow8BitIntegers);
    }
    auto res = ir::Validate(mod, caps);
    if (int8_allowed) {
        ASSERT_EQ(res, Success) << res.Failure();
    } else {
        ASSERT_NE(res, Success);
        EXPECT_THAT(res.Failure().reason.Str(),
                    testing::HasSubstr("8-bit integer types are not permitted"));
    }
}

INSTANTIATE_TEST_SUITE_P(Int8Types,
                         IR_Validator8BitIntTypeTest,
                         testing::Combine(
                             /* int8_allowed */ testing::Values(false, true),
                             /* type_builder */
                             testing::Values(TypeBuilder<i8>,
                                             TypeBuilder<u8>,
                                             TypeBuilder<vec4<i8>>,
                                             TypeBuilder<array<u8, 4>>)));

TEST_F(IR_ValidatorTest, Int8Type_InstructionOperand_NotAllowed) {
    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Convert(ty.i32(), u8(1));
        b.Return(fn);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:22 error: convert: 8-bit integer types are not permitted
    %2:i32 = convert 1u8
                     ^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:i32 = convert 1u8
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Int8Type_InstructionOperand_Allowed) {
    auto* fn = b.Function("my_func", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Convert(ty.i32(), u8(1));
        b.Return(fn);
    });

    auto res = ir::Validate(mod, Capabilities{Capability::kAllow8BitIntegers});
    ASSERT_EQ(res, Success) << res.Failure();
}

TEST_F(IR_ValidatorTest, Switch_NoCondition) {
    auto* f = b.Function("my_func", ty.void_());

    auto* s = b.ir.CreateInstruction<ir::Switch>();
    f->Block()->Append(s);
    b.Append(b.DefaultCase(s), [&] { b.ExitSwitch(s); });
    f->Block()->Append(b.Return(f));

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(), R"(error: switch: operand is undefined
:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    switch undef [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Switch_ConditionPointer) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* s = b.Switch(b.Var("a", b.Zero<i32>()));
        b.Append(b.DefaultCase(s), [&] { b.ExitSwitch(s); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(error: switch: condition type must be an integer scalar
:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %a:ptr<function, i32, read_write> = var, 0i
    switch %a [c: (default, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Switch_NoCases) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        b.Switch(1_i);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: switch: missing default case for switch
    switch 1i [] {  # switch_1
    ^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    switch 1i [] {  # switch_1
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Switch_NoDefaultCase) {
    auto* f = b.Function("my_func", ty.void_());

    b.Append(f->Block(), [&] {
        auto* s = b.Switch(1_i);
        b.Append(b.Case(s, {b.Constant(0_i)}), [&] { b.ExitSwitch(s); });
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:5 error: switch: missing default case for switch
    switch 1i [c: (0i, $B2)] {  # switch_1
    ^^^^^^^^^^^^^^^^^^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    switch 1i [c: (0i, $B2)] {  # switch_1
      $B2: {  # case
        exit_switch  # switch_1
      }
    }
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Swizzle_MissingValue) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), var, {3, 2, 1, 0});
        swizzle->ClearOperands();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:20 error: swizzle: expected exactly 1 operands, got 0
    %3:vec4<f32> = swizzle undef, wzyx
                   ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec4<f32>, read_write> = var
    %3:vec4<f32> = swizzle undef, wzyx
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Swizzle_NullValue) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), var, {3, 2, 1, 0});
        swizzle->SetOperand(0, nullptr);
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(error: swizzle: operand is undefined
:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec4<f32>, read_write> = var
    %3:vec4<f32> = swizzle undef, wzyx
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Swizzle_MissingResult) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), var, {3, 2, 1, 0});
        swizzle->ClearResults();
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:13 error: swizzle: expected exactly 1 results, got 0
    undef = swizzle %2, wzyx
            ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec4<f32>, read_write> = var
    undef = swizzle %2, wzyx
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Swizzle_NullResult) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), var, {3, 2, 1, 0});
        swizzle->SetResults(Vector<ir::InstructionResult*, 1>{nullptr});
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:5 error: swizzle: result is undefined
    undef = swizzle %2, wzyx
    ^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec4<f32>, read_write> = var
    undef = swizzle %2, wzyx
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Swizzle_NoIndices) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), var, {3, 2, 1, 0});
        auto indices = Vector<uint32_t, 0>();
        swizzle->SetIndices(std::move(indices));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:20 error: swizzle: expected at least 1 indices
    %3:vec4<f32> = swizzle %2,
                   ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec4<f32>, read_write> = var
    %3:vec4<f32> = swizzle %2,
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Swizzle_TooManyIndices) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), var, {3, 2, 1, 0});
        auto indices = Vector<uint32_t, 5>{1, 1, 1, 1, 1};
        swizzle->SetIndices(std::move(indices));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:20 error: swizzle: expected at most 4 indices
    %3:vec4<f32> = swizzle %2, yyyyy
                   ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec4<f32>, read_write> = var
    %3:vec4<f32> = swizzle %2, yyyyy
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, Swizzle_InvalidIndices) {
    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        auto* var = b.Var(ty.ptr(function, ty.vec4<f32>()));
        auto* swizzle = b.Swizzle(ty.vec4<f32>(), var, {3, 2, 1, 0});
        auto indices = Vector<uint32_t, 4>{4, 3, 2, 1};
        swizzle->SetIndices(std::move(indices));
        b.Return(f);
    });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:4:20 error: swizzle: invalid index value
    %3:vec4<f32> = swizzle %2, wzy
                   ^^^^^^^

:2:3 note: in block
  $B1: {
  ^^^

note: # Disassembly
%my_func = func():void {
  $B1: {
    %2:ptr<function, vec4<f32>, read_write> = var
    %3:vec4<f32> = swizzle %2, wzy
    ret
  }
}
)");
}

TEST_F(IR_ValidatorTest, OverrideWithoutCapability) {
    b.Append(mod.root_block, [&] { b.Override("a", 1_u); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:12 error: override: root block: invalid instruction: tint::core::ir::Override
  %a:u32 = override, 1u @id(0)
           ^^^^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %a:u32 = override, 1u @id(0)
}

)");
}

TEST_F(IR_ValidatorTest, InstructionInRootBlockWithoutOverrideCap) {
    b.Append(mod.root_block, [&] { b.Add(ty.u32(), 3_u, 2_u); });

    auto res = ir::Validate(mod);
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:3 error: binary: root block: invalid instruction: tint::core::ir::CoreBinary
  %1:u32 = add 3u, 2u
  ^^^^^^^^^^^^^^^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:u32 = add 3u, 2u
}

)");
}

TEST_F(IR_ValidatorTest, OverrideWithCapability) {
    b.Append(mod.root_block, [&] { b.Override(ty.u32()); });

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, OverrideWithValue) {
    b.Append(mod.root_block, [&] {
        auto* z = b.Override(ty.u32());
        z->SetOverrideId(OverrideId{2});
        auto* init = b.Add(ty.u32(), z, 2_u);

        b.Override("a", init);
    });

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_EQ(res, Success);
}

TEST_F(IR_ValidatorTest, OverrideWithInvalidType) {
    b.Append(mod.root_block, [&] { b.Override(ty.vec3<u32>()); });

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:18 error: override: override type 'vec3<u32>' is not a scalar
  %1:vec3<u32> = override @id(0)
                 ^^^^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:vec3<u32> = override @id(0)
}

)");
}

TEST_F(IR_ValidatorTest, OverrideWithMismatchedInitializerType) {
    b.Append(mod.root_block, [&] {
        auto* init = b.Constant(1_i);
        auto* o = b.Override(ty.u32());
        o->SetInitializer(init);
    });

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:2:12 error: override: override type 'u32' does not match initializer type 'i32'
  %1:u32 = override, 1i @id(0)
           ^^^^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:u32 = override, 1i @id(0)
}

)");
}

TEST_F(IR_ValidatorTest, OverrideDuplicateId) {
    b.Append(mod.root_block, [&] {
        auto* o = b.Override(ty.u32());
        o->SetOverrideId(OverrideId{2});

        auto* o2 = b.Override(ty.i32());
        o2->SetOverrideId(OverrideId{2});
    });

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason.Str(),
              R"(:3:12 error: override: duplicate override id encountered: 2
  %2:i32 = override @id(2)
           ^^^^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:u32 = override @id(2)
  %2:i32 = override @id(2)
}

)");
}

TEST_F(IR_ValidatorTest, InstructionInRootBlockOnlyUsedInRootBlock) {
    core::ir::Value* init = nullptr;
    b.Append(mod.root_block, [&] {
        auto* z = b.Override(ty.u32());
        z->SetOverrideId(OverrideId{2});
        init = b.Add(ty.u32(), z, 2_u)->Result(0);
        b.Override("a", init);
    });

    auto* f = b.Function("my_func", ty.void_());
    b.Append(f->Block(), [&] {
        b.Add(ty.u32(), init, 2_u);
        b.Return(f);
    });

    auto res = ir::Validate(mod, core::ir::Capabilities{core::ir::Capability::kAllowOverrides});
    ASSERT_NE(res, Success);
    EXPECT_EQ(
        res.Failure().reason.Str(),
        R"(:3:3 error: binary: root block: instruction used outside of root block tint::core::ir::CoreBinary
  %2:u32 = add %1, 2u
  ^^^^^^^^^^^^^^^^^^^

:1:1 note: in block
$B1: {  # root
^^^

note: # Disassembly
$B1: {  # root
  %1:u32 = override @id(2)
  %2:u32 = add %1, 2u
  %a:u32 = override, %2 @id(0)
}

%my_func = func():void {
  $B2: {
    %5:u32 = add %2, 2u
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::core::ir
