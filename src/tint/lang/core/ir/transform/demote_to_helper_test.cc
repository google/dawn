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

#include "src/tint/lang/core/ir/transform/demote_to_helper.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::core::ir::transform {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using IR_DemoteToHelperTest = TransformTest;

TEST_F(IR_DemoteToHelperTest, NoModify_NoDiscard) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetReturnLocation(0_u, {});

    b.Append(ep->Block(), [&] {  //
        b.Store(buffer, 42_i);
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
%b1 = block {  # root
  %buffer:ptr<storage, i32, read_write> = var @binding_point(0, 0)
}

%ep = @fragment func():f32 [@location(0)] -> %b2 {
  %b2 = block {
    store %buffer, 42i
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = src;

    Run(DemoteToHelper);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, DiscardInEntryPoint_WriteInEntryPoint) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(FunctionParam::Builtin::kFrontFacing);
    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetParams({front_facing});
    ep->SetReturnLocation(0_u, {});

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {  //
            b.Discard();
            b.ExitIf(ifelse);
        });
        b.Store(buffer, 42_i);
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
%b1 = block {  # root
  %buffer:ptr<storage, i32, read_write> = var @binding_point(0, 0)
}

%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        discard
        exit_if  # if_1
      }
    }
    store %buffer, 42i
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %buffer:ptr<storage, i32, read_write> = var @binding_point(0, 0)
  %continue_execution:ptr<private, bool, read_write> = var, true
}

%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        store %continue_execution, false
        exit_if  # if_1
      }
    }
    %5:bool = load %continue_execution
    if %5 [t: %b4] {  # if_2
      %b4 = block {  # true
        store %buffer, 42i
        exit_if  # if_2
      }
    }
    %6:bool = load %continue_execution
    %7:bool = eq %6, false
    if %7 [t: %b5] {  # if_3
      %b5 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run(DemoteToHelper);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, DiscardInEntryPoint_WriteInHelper) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* helper = b.Function("foo", ty.void_());
    b.Append(helper->Block(), [&] {
        b.Store(buffer, 42_i);
        b.Return(helper);
    });

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(FunctionParam::Builtin::kFrontFacing);
    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetParams({front_facing});
    ep->SetReturnLocation(0_u, {});

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {  //
            b.Discard();
            b.ExitIf(ifelse);
        });
        b.Call(ty.void_(), helper);
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
%b1 = block {  # root
  %buffer:ptr<storage, i32, read_write> = var @binding_point(0, 0)
}

%foo = func():void -> %b2 {
  %b2 = block {
    store %buffer, 42i
    ret
  }
}
%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b3 {
  %b3 = block {
    if %front_facing [t: %b4] {  # if_1
      %b4 = block {  # true
        discard
        exit_if  # if_1
      }
    }
    %5:void = call %foo
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %buffer:ptr<storage, i32, read_write> = var @binding_point(0, 0)
  %continue_execution:ptr<private, bool, read_write> = var, true
}

%foo = func():void -> %b2 {
  %b2 = block {
    %4:bool = load %continue_execution
    if %4 [t: %b3] {  # if_1
      %b3 = block {  # true
        store %buffer, 42i
        exit_if  # if_1
      }
    }
    ret
  }
}
%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b4 {
  %b4 = block {
    if %front_facing [t: %b5] {  # if_2
      %b5 = block {  # true
        store %continue_execution, false
        exit_if  # if_2
      }
    }
    %7:void = call %foo
    %8:bool = load %continue_execution
    %9:bool = eq %8, false
    if %9 [t: %b6] {  # if_3
      %b6 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run(DemoteToHelper);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, DiscardInHelper_WriteInEntryPoint) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* helper = b.Function("foo", ty.void_());
    helper->SetParams({cond});
    b.Append(helper->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Append(ifelse->True(), [&] {  //
            b.Discard();
            b.ExitIf(ifelse);
        });
        b.Return(helper);
    });

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(FunctionParam::Builtin::kFrontFacing);
    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetParams({front_facing});
    ep->SetReturnLocation(0_u, {});

    b.Append(ep->Block(), [&] {
        b.Call(ty.void_(), helper, front_facing);
        b.Store(buffer, 42_i);
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
%b1 = block {  # root
  %buffer:ptr<storage, i32, read_write> = var @binding_point(0, 0)
}

%foo = func(%cond:bool):void -> %b2 {
  %b2 = block {
    if %cond [t: %b3] {  # if_1
      %b3 = block {  # true
        discard
        exit_if  # if_1
      }
    }
    ret
  }
}
%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b4 {
  %b4 = block {
    %6:void = call %foo, %front_facing
    store %buffer, 42i
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %buffer:ptr<storage, i32, read_write> = var @binding_point(0, 0)
  %continue_execution:ptr<private, bool, read_write> = var, true
}

%foo = func(%cond:bool):void -> %b2 {
  %b2 = block {
    if %cond [t: %b3] {  # if_1
      %b3 = block {  # true
        store %continue_execution, false
        exit_if  # if_1
      }
    }
    ret
  }
}
%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b4 {
  %b4 = block {
    %7:void = call %foo, %front_facing
    %8:bool = load %continue_execution
    if %8 [t: %b5] {  # if_2
      %b5 = block {  # true
        store %buffer, 42i
        exit_if  # if_2
      }
    }
    %9:bool = load %continue_execution
    %10:bool = eq %9, false
    if %10 [t: %b6] {  # if_3
      %b6 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run(DemoteToHelper);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, DiscardInHelper_WriteInHelper) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* cond = b.FunctionParam("cond", ty.bool_());
    auto* helper = b.Function("foo", ty.void_());
    helper->SetParams({cond});
    b.Append(helper->Block(), [&] {
        auto* ifelse = b.If(cond);
        b.Append(ifelse->True(), [&] {  //
            b.Discard();
            b.ExitIf(ifelse);
        });
        b.Store(buffer, 42_i);
        b.Return(helper);
    });

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(FunctionParam::Builtin::kFrontFacing);
    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetParams({front_facing});
    ep->SetReturnLocation(0_u, {});

    b.Append(ep->Block(), [&] {
        b.Call(ty.void_(), helper, front_facing);
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
%b1 = block {  # root
  %buffer:ptr<storage, i32, read_write> = var @binding_point(0, 0)
}

%foo = func(%cond:bool):void -> %b2 {
  %b2 = block {
    if %cond [t: %b3] {  # if_1
      %b3 = block {  # true
        discard
        exit_if  # if_1
      }
    }
    store %buffer, 42i
    ret
  }
}
%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b4 {
  %b4 = block {
    %6:void = call %foo, %front_facing
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %buffer:ptr<storage, i32, read_write> = var @binding_point(0, 0)
  %continue_execution:ptr<private, bool, read_write> = var, true
}

%foo = func(%cond:bool):void -> %b2 {
  %b2 = block {
    if %cond [t: %b3] {  # if_1
      %b3 = block {  # true
        store %continue_execution, false
        exit_if  # if_1
      }
    }
    %5:bool = load %continue_execution
    if %5 [t: %b4] {  # if_2
      %b4 = block {  # true
        store %buffer, 42i
        exit_if  # if_2
      }
    }
    ret
  }
}
%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b5 {
  %b5 = block {
    %8:void = call %foo, %front_facing
    %9:bool = load %continue_execution
    %10:bool = eq %9, false
    if %10 [t: %b6] {  # if_3
      %b6 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run(DemoteToHelper);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, WriteToInvocationPrivateAddressSpace) {
    auto* priv = mod.root_block->Append(b.Var("priv", ty.ptr<private_, i32>()));
    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(FunctionParam::Builtin::kFrontFacing);
    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetParams({front_facing});
    ep->SetReturnLocation(0_u, {});

    b.Append(ep->Block(), [&] {
        auto* func = b.Var("func", ty.ptr<function, i32>());
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {  //
            b.Discard();
            b.ExitIf(ifelse);
        });
        b.Store(priv, 42_i);
        b.Store(func, 42_i);
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
%b1 = block {  # root
  %priv:ptr<private, i32, read_write> = var
}

%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b2 {
  %b2 = block {
    %func:ptr<function, i32, read_write> = var
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        discard
        exit_if  # if_1
      }
    }
    store %priv, 42i
    store %func, 42i
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %priv:ptr<private, i32, read_write> = var
  %continue_execution:ptr<private, bool, read_write> = var, true
}

%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b2 {
  %b2 = block {
    %func:ptr<function, i32, read_write> = var
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        store %continue_execution, false
        exit_if  # if_1
      }
    }
    store %priv, 42i
    store %func, 42i
    %6:bool = load %continue_execution
    %7:bool = eq %6, false
    if %7 [t: %b4] {  # if_2
      %b4 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run(DemoteToHelper);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, TextureStore) {
    auto format = core::TexelFormat::kR32Float;
    auto* texture =
        b.Var("texture", ty.ptr(core::AddressSpace::kHandle,
                                ty.Get<core::type::StorageTexture>(
                                    core::type::TextureDimension::k2d, format, core::Access::kWrite,
                                    core::type::StorageTexture::SubtypeFor(format, ty))));
    texture->SetBindingPoint(0, 0);
    mod.root_block->Append(texture);

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(FunctionParam::Builtin::kFrontFacing);
    auto* coord = b.FunctionParam("coord", ty.vec2<i32>());
    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetParams({front_facing, coord});
    ep->SetReturnLocation(0_u, {});

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {  //
            b.Discard();
            b.ExitIf(ifelse);
        });
        b.Call(ty.void_(), core::BuiltinFn::kTextureStore, b.Load(texture), coord,
               b.Splat(b.ir.Types().vec4<f32>(), 0.5_f, 4));
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_2d<r32float, write>, read_write> = var @binding_point(0, 0)
}

%ep = @fragment func(%front_facing:bool [@front_facing], %coord:vec2<i32>):f32 [@location(0)] -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        discard
        exit_if  # if_1
      }
    }
    %5:texture_storage_2d<r32float, write> = load %texture
    %6:void = textureStore %5, %coord, vec4<f32>(0.5f)
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %texture:ptr<handle, texture_storage_2d<r32float, write>, read_write> = var @binding_point(0, 0)
  %continue_execution:ptr<private, bool, read_write> = var, true
}

%ep = @fragment func(%front_facing:bool [@front_facing], %coord:vec2<i32>):f32 [@location(0)] -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        store %continue_execution, false
        exit_if  # if_1
      }
    }
    %6:texture_storage_2d<r32float, write> = load %texture
    %7:bool = load %continue_execution
    if %7 [t: %b4] {  # if_2
      %b4 = block {  # true
        %8:void = textureStore %6, %coord, vec4<f32>(0.5f)
        exit_if  # if_2
      }
    }
    %9:bool = load %continue_execution
    %10:bool = eq %9, false
    if %10 [t: %b5] {  # if_3
      %b5 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run(DemoteToHelper);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, AtomicStore) {
    auto* buffer = b.Var("buffer", ty.ptr(storage, ty.atomic<i32>()));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(FunctionParam::Builtin::kFrontFacing);
    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetParams({front_facing});
    ep->SetReturnLocation(0_u, {});

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {  //
            b.Discard();
            b.ExitIf(ifelse);
        });
        b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, buffer, 42_i);
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
%b1 = block {  # root
  %buffer:ptr<storage, atomic<i32>, read_write> = var @binding_point(0, 0)
}

%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        discard
        exit_if  # if_1
      }
    }
    %4:void = atomicStore %buffer, 42i
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %buffer:ptr<storage, atomic<i32>, read_write> = var @binding_point(0, 0)
  %continue_execution:ptr<private, bool, read_write> = var, true
}

%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        store %continue_execution, false
        exit_if  # if_1
      }
    }
    %5:bool = load %continue_execution
    if %5 [t: %b4] {  # if_2
      %b4 = block {  # true
        %6:void = atomicStore %buffer, 42i
        exit_if  # if_2
      }
    }
    %7:bool = load %continue_execution
    %8:bool = eq %7, false
    if %8 [t: %b5] {  # if_3
      %b5 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run(DemoteToHelper);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, AtomicAdd) {
    auto* buffer = b.Var("buffer", ty.ptr(storage, ty.atomic<i32>()));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(FunctionParam::Builtin::kFrontFacing);
    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetParams({front_facing});
    ep->SetReturnLocation(0_u, {});

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {  //
            b.Discard();
            b.ExitIf(ifelse);
        });
        auto* old = b.Call(ty.i32(), core::BuiltinFn::kAtomicAdd, buffer, 42_i);
        b.Add(ty.i32(), old, 1_i);
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
%b1 = block {  # root
  %buffer:ptr<storage, atomic<i32>, read_write> = var @binding_point(0, 0)
}

%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        discard
        exit_if  # if_1
      }
    }
    %4:i32 = atomicAdd %buffer, 42i
    %5:i32 = add %4, 1i
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %buffer:ptr<storage, atomic<i32>, read_write> = var @binding_point(0, 0)
  %continue_execution:ptr<private, bool, read_write> = var, true
}

%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        store %continue_execution, false
        exit_if  # if_1
      }
    }
    %5:bool = load %continue_execution
    %6:i32 = if %5 [t: %b4] {  # if_2
      %b4 = block {  # true
        %7:i32 = atomicAdd %buffer, 42i
        exit_if %7  # if_2
      }
      # implicit false block: exit_if undef
    }
    %8:i32 = add %6, 1i
    %9:bool = load %continue_execution
    %10:bool = eq %9, false
    if %10 [t: %b5] {  # if_3
      %b5 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run(DemoteToHelper);

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, AtomicCompareExchange) {
    auto* buffer = b.Var("buffer", ty.ptr(storage, ty.atomic<i32>()));
    buffer->SetBindingPoint(0, 0);
    mod.root_block->Append(buffer);

    auto* front_facing = b.FunctionParam("front_facing", ty.bool_());
    front_facing->SetBuiltin(FunctionParam::Builtin::kFrontFacing);
    auto* ep = b.Function("ep", ty.f32(), Function::PipelineStage::kFragment);
    ep->SetParams({front_facing});
    ep->SetReturnLocation(0_u, {});

    b.Append(ep->Block(), [&] {
        auto* ifelse = b.If(front_facing);
        b.Append(ifelse->True(), [&] {  //
            b.Discard();
            b.ExitIf(ifelse);
        });
        auto* result =
            b.Call(core::type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                   core::BuiltinFn::kAtomicCompareExchangeWeak, buffer, 0_i, 42_i);
        b.Add(ty.i32(), b.Access(ty.i32(), result, 0_i), 1_i);
        b.Return(ep, 0.5_f);
    });

    auto* src = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%b1 = block {  # root
  %buffer:ptr<storage, atomic<i32>, read_write> = var @binding_point(0, 0)
}

%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        discard
        exit_if  # if_1
      }
    }
    %4:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %buffer, 0i, 42i
    %5:i32 = access %4, 0i
    %6:i32 = add %5, 1i
    ret 0.5f
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
__atomic_compare_exchange_result_i32 = struct @align(4) {
  old_value:i32 @offset(0)
  exchanged:bool @offset(4)
}

%b1 = block {  # root
  %buffer:ptr<storage, atomic<i32>, read_write> = var @binding_point(0, 0)
  %continue_execution:ptr<private, bool, read_write> = var, true
}

%ep = @fragment func(%front_facing:bool [@front_facing]):f32 [@location(0)] -> %b2 {
  %b2 = block {
    if %front_facing [t: %b3] {  # if_1
      %b3 = block {  # true
        store %continue_execution, false
        exit_if  # if_1
      }
    }
    %5:bool = load %continue_execution
    %6:__atomic_compare_exchange_result_i32 = if %5 [t: %b4] {  # if_2
      %b4 = block {  # true
        %7:__atomic_compare_exchange_result_i32 = atomicCompareExchangeWeak %buffer, 0i, 42i
        exit_if %7  # if_2
      }
      # implicit false block: exit_if undef
    }
    %8:i32 = access %6, 0i
    %9:i32 = add %8, 1i
    %10:bool = load %continue_execution
    %11:bool = eq %10, false
    if %11 [t: %b5] {  # if_3
      %b5 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run(DemoteToHelper);

    EXPECT_EQ(expect, str());
}

// Test that we transform unreachable functions that discard (see crbug.com/tint/2052).
TEST_F(IR_DemoteToHelperTest, UnreachableHelperThatDiscards) {
    auto* helper = b.Function("foo", ty.void_());
    b.Append(helper->Block(), [&] {
        b.Discard();
        b.Return(helper);
    });

    auto* src = R"(
%foo = func():void -> %b1 {
  %b1 = block {
    discard
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %continue_execution:ptr<private, bool, read_write> = var, true
}

%foo = func():void -> %b2 {
  %b2 = block {
    store %continue_execution, false
    ret
  }
}
)";

    Run(DemoteToHelper);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::core::ir::transform
