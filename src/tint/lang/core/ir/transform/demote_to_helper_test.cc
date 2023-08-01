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

#include "src/tint/lang/core/ir/transform/demote_to_helper.h"

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::ir::transform {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

using IR_DemoteToHelperTest = TransformTest;

TEST_F(IR_DemoteToHelperTest, NoModify_NoDiscard) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

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

    Run<DemoteToHelper>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, DiscardInEntryPoint_WriteInEntryPoint) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

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
    if %6 [t: %b5] {  # if_3
      %b5 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run<DemoteToHelper>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, DiscardInEntryPoint_WriteInHelper) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

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
    if %8 [t: %b6] {  # if_3
      %b6 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run<DemoteToHelper>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, DiscardInHelper_WriteInEntryPoint) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

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
    if %9 [t: %b6] {  # if_3
      %b6 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run<DemoteToHelper>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, DiscardInHelper_WriteInHelper) {
    auto* buffer = b.Var("buffer", ty.ptr<storage, i32>());
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

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
    if %9 [t: %b6] {  # if_3
      %b6 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run<DemoteToHelper>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, WriteToInvocationPrivateAddressSpace) {
    auto* priv = b.RootBlock()->Append(b.Var("priv", ty.ptr<private_, i32>()));
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
    if %6 [t: %b4] {  # if_2
      %b4 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run<DemoteToHelper>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, TextureStore) {
    auto format = builtin::TexelFormat::kR32Float;
    auto* texture =
        b.Var("texture", ty.ptr(builtin::AddressSpace::kHandle,
                                ty.Get<type::StorageTexture>(
                                    type::TextureDimension::k2d, format, builtin::Access::kWrite,
                                    type::StorageTexture::SubtypeFor(format, ty))));
    texture->SetBindingPoint(0, 0);
    b.RootBlock()->Append(texture);

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
        b.Call(ty.void_(), builtin::Function::kTextureStore, texture, coord, 0.5_f);
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
    %5:void = textureStore %texture, %coord, 0.5f
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
    %6:bool = load %continue_execution
    if %6 [t: %b4] {  # if_2
      %b4 = block {  # true
        %7:void = textureStore %texture, %coord, 0.5f
        exit_if  # if_2
      }
    }
    %8:bool = load %continue_execution
    if %8 [t: %b5] {  # if_3
      %b5 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run<DemoteToHelper>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, AtomicStore) {
    auto* buffer = b.Var("buffer", ty.ptr(storage, ty.atomic<i32>()));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

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
        b.Call(ty.void_(), builtin::Function::kAtomicStore, buffer, 42_i);
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
    if %7 [t: %b5] {  # if_3
      %b5 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run<DemoteToHelper>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, AtomicAdd) {
    auto* buffer = b.Var("buffer", ty.ptr(storage, ty.atomic<i32>()));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

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
        auto* old = b.Call(ty.i32(), builtin::Function::kAtomicAdd, buffer, 42_i);
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
    if %9 [t: %b5] {  # if_3
      %b5 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run<DemoteToHelper>();

    EXPECT_EQ(expect, str());
}

TEST_F(IR_DemoteToHelperTest, AtomicCompareExchange) {
    auto* buffer = b.Var("buffer", ty.ptr(storage, ty.atomic<i32>()));
    buffer->SetBindingPoint(0, 0);
    b.RootBlock()->Append(buffer);

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
        auto* result = b.Call(type::CreateAtomicCompareExchangeResult(ty, mod.symbols, ty.i32()),
                              builtin::Function::kAtomicCompareExchangeWeak, buffer, 0_i, 42_i);
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
    if %10 [t: %b5] {  # if_3
      %b5 = block {  # true
        terminate_invocation
      }
    }
    ret 0.5f
  }
}
)";

    Run<DemoteToHelper>();

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::ir::transform
