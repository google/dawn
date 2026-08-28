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

#include "src/tint/lang/spirv/reader/lower/builtins.h"

#include <string>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

struct SpirvReader_BuiltinsTest : public core::ir::transform::TransformTest {
  protected:
    void SetUp() override { mod.properties.Add(core::ir::Property::kAllow16BitFloats); }
};

TEST_F(SpirvReader_BuiltinsTest, Normalize_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kNormalize, 10_f);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.normalize 10.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = sign 10.0f
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Normalize_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kNormalize,
                                       b.Splat(ty.vec2f(), 10_f));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.normalize vec2<f32>(10.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = normalize vec2<f32>(10.0f)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Inverse_Mat2x2f) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(
            ty.mat2x2<f32>(), spirv::BuiltinFn::kInverse,
            b.Construct(ty.mat2x2<f32>(), b.Splat(ty.vec2f(), 10_f), b.Splat(ty.vec2f(), 20_f)));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x2<f32> = construct vec2<f32>(10.0f), vec2<f32>(20.0f)
    %3:mat2x2<f32> = spirv.inverse %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x2<f32> = construct vec2<f32>(10.0f), vec2<f32>(20.0f)
    %3:f32 = determinant %2
    %4:f32 = div 1.0f, %3
    %5:f32 = negation %4
    %6:f32 = access %2, 0u, 0u
    %7:f32 = access %2, 0u, 1u
    %8:f32 = access %2, 1u, 0u
    %9:f32 = access %2, 1u, 1u
    %10:f32 = mul %4, %9
    %11:f32 = mul %5, %7
    %12:f32 = mul %5, %8
    %13:f32 = mul %4, %6
    %14:vec2<f32> = construct %10, %11
    %15:vec2<f32> = construct %12, %13
    %16:mat2x2<f32> = construct %14, %15
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Inverse_Mat2x2h) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(
            ty.mat2x2<f16>(), spirv::BuiltinFn::kInverse,
            b.Construct(ty.mat2x2<f16>(), b.Splat(ty.vec2h(), 10_h), b.Splat(ty.vec2h(), 20_h)));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x2<f16> = construct vec2<f16>(10.0h), vec2<f16>(20.0h)
    %3:mat2x2<f16> = spirv.inverse %2
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat2x2<f16> = construct vec2<f16>(10.0h), vec2<f16>(20.0h)
    %3:f16 = determinant %2
    %4:f16 = div 1.0h, %3
    %5:f16 = negation %4
    %6:f16 = access %2, 0u, 0u
    %7:f16 = access %2, 0u, 1u
    %8:f16 = access %2, 1u, 0u
    %9:f16 = access %2, 1u, 1u
    %10:f16 = mul %4, %9
    %11:f16 = mul %5, %7
    %12:f16 = mul %5, %8
    %13:f16 = mul %4, %6
    %14:vec2<f16> = construct %10, %11
    %15:vec2<f16> = construct %12, %13
    %16:mat2x2<f16> = construct %14, %15
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Inverse_Mat3x3f) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* m = b.Let("m", b.Construct(ty.mat3x3<f32>(), b.Splat(ty.vec3f(), 10_f),
                                         b.Splat(ty.vec3f(), 20_f), b.Splat(ty.vec3f(), 30_f)));
        b.Call<spirv::ir::BuiltinCall>(ty.mat3x3<f32>(), spirv::BuiltinFn::kInverse, m);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat3x3<f32> = construct vec3<f32>(10.0f), vec3<f32>(20.0f), vec3<f32>(30.0f)
    %m:mat3x3<f32> = let %2
    %4:mat3x3<f32> = spirv.inverse %m
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat3x3<f32> = construct vec3<f32>(10.0f), vec3<f32>(20.0f), vec3<f32>(30.0f)
    %m:mat3x3<f32> = let %2
    %4:f32 = determinant %m
    %5:f32 = div 1.0f, %4
    %6:f32 = access %m, 0u, 0u
    %7:f32 = access %m, 0u, 1u
    %8:f32 = access %m, 0u, 2u
    %9:f32 = access %m, 1u, 0u
    %10:f32 = access %m, 1u, 1u
    %11:f32 = access %m, 1u, 2u
    %12:f32 = access %m, 2u, 0u
    %13:f32 = access %m, 2u, 1u
    %14:f32 = access %m, 2u, 2u
    %15:f32 = mul %10, %14
    %16:f32 = mul %11, %13
    %17:f32 = sub %15, %16
    %18:f32 = mul %8, %13
    %19:f32 = mul %7, %14
    %20:f32 = sub %18, %19
    %21:f32 = mul %7, %11
    %22:f32 = mul %8, %10
    %23:f32 = sub %21, %22
    %24:f32 = mul %11, %12
    %25:f32 = mul %9, %14
    %26:f32 = sub %24, %25
    %27:f32 = mul %6, %14
    %28:f32 = mul %8, %12
    %29:f32 = sub %27, %28
    %30:f32 = mul %8, %9
    %31:f32 = mul %6, %11
    %32:f32 = sub %30, %31
    %33:f32 = mul %9, %13
    %34:f32 = mul %10, %12
    %35:f32 = sub %33, %34
    %36:f32 = mul %7, %12
    %37:f32 = mul %6, %13
    %38:f32 = sub %36, %37
    %39:f32 = mul %6, %10
    %40:f32 = mul %7, %9
    %41:f32 = sub %39, %40
    %42:vec3<f32> = construct %17, %20, %23
    %43:vec3<f32> = construct %26, %29, %32
    %44:vec3<f32> = construct %35, %38, %41
    %45:mat3x3<f32> = construct %42, %43, %44
    %46:mat3x3<f32> = mul %5, %45
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Inverse_Mat3x3h) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* m = b.Let("m", b.Construct(ty.mat3x3<f16>(), b.Splat(ty.vec3h(), 10_h),
                                         b.Splat(ty.vec3h(), 20_h), b.Splat(ty.vec3h(), 30_h)));
        b.Call<spirv::ir::BuiltinCall>(ty.mat3x3<f16>(), spirv::BuiltinFn::kInverse, m);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat3x3<f16> = construct vec3<f16>(10.0h), vec3<f16>(20.0h), vec3<f16>(30.0h)
    %m:mat3x3<f16> = let %2
    %4:mat3x3<f16> = spirv.inverse %m
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat3x3<f16> = construct vec3<f16>(10.0h), vec3<f16>(20.0h), vec3<f16>(30.0h)
    %m:mat3x3<f16> = let %2
    %4:f16 = determinant %m
    %5:f16 = div 1.0h, %4
    %6:f16 = access %m, 0u, 0u
    %7:f16 = access %m, 0u, 1u
    %8:f16 = access %m, 0u, 2u
    %9:f16 = access %m, 1u, 0u
    %10:f16 = access %m, 1u, 1u
    %11:f16 = access %m, 1u, 2u
    %12:f16 = access %m, 2u, 0u
    %13:f16 = access %m, 2u, 1u
    %14:f16 = access %m, 2u, 2u
    %15:f16 = mul %10, %14
    %16:f16 = mul %11, %13
    %17:f16 = sub %15, %16
    %18:f16 = mul %8, %13
    %19:f16 = mul %7, %14
    %20:f16 = sub %18, %19
    %21:f16 = mul %7, %11
    %22:f16 = mul %8, %10
    %23:f16 = sub %21, %22
    %24:f16 = mul %11, %12
    %25:f16 = mul %9, %14
    %26:f16 = sub %24, %25
    %27:f16 = mul %6, %14
    %28:f16 = mul %8, %12
    %29:f16 = sub %27, %28
    %30:f16 = mul %8, %9
    %31:f16 = mul %6, %11
    %32:f16 = sub %30, %31
    %33:f16 = mul %9, %13
    %34:f16 = mul %10, %12
    %35:f16 = sub %33, %34
    %36:f16 = mul %7, %12
    %37:f16 = mul %6, %13
    %38:f16 = sub %36, %37
    %39:f16 = mul %6, %10
    %40:f16 = mul %7, %9
    %41:f16 = sub %39, %40
    %42:vec3<f16> = construct %17, %20, %23
    %43:vec3<f16> = construct %26, %29, %32
    %44:vec3<f16> = construct %35, %38, %41
    %45:mat3x3<f16> = construct %42, %43, %44
    %46:mat3x3<f16> = mul %5, %45
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Inverse_Mat4x4f) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* m = b.Let(
            "m", b.Construct(ty.mat4x4<f32>(), b.Splat(ty.vec4f(), 10_f), b.Splat(ty.vec4f(), 20_f),
                             b.Splat(ty.vec4f(), 30_f), b.Splat(ty.vec4f(), 40_f)));
        b.Call<spirv::ir::BuiltinCall>(ty.mat4x4<f32>(), spirv::BuiltinFn::kInverse, m);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat4x4<f32> = construct vec4<f32>(10.0f), vec4<f32>(20.0f), vec4<f32>(30.0f), vec4<f32>(40.0f)
    %m:mat4x4<f32> = let %2
    %4:mat4x4<f32> = spirv.inverse %m
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat4x4<f32> = construct vec4<f32>(10.0f), vec4<f32>(20.0f), vec4<f32>(30.0f), vec4<f32>(40.0f)
    %m:mat4x4<f32> = let %2
    %4:f32 = determinant %m
    %5:f32 = div 1.0f, %4
    %6:f32 = access %m, 0u, 0u
    %7:f32 = access %m, 0u, 1u
    %8:f32 = access %m, 0u, 2u
    %9:f32 = access %m, 0u, 3u
    %10:f32 = access %m, 1u, 0u
    %11:f32 = access %m, 1u, 1u
    %12:f32 = access %m, 1u, 2u
    %13:f32 = access %m, 1u, 3u
    %14:f32 = access %m, 2u, 0u
    %15:f32 = access %m, 2u, 1u
    %16:f32 = access %m, 2u, 2u
    %17:f32 = access %m, 2u, 3u
    %18:f32 = access %m, 3u, 0u
    %19:f32 = access %m, 3u, 1u
    %20:f32 = access %m, 3u, 2u
    %21:f32 = access %m, 3u, 3u
    %22:f32 = mul %16, %21
    %23:f32 = mul %17, %20
    %24:f32 = sub %22, %23
    %25:f32 = mul %15, %21
    %26:f32 = mul %17, %19
    %27:f32 = sub %25, %26
    %28:f32 = mul %15, %20
    %29:f32 = mul %16, %19
    %30:f32 = sub %28, %29
    %31:f32 = mul %12, %21
    %32:f32 = mul %13, %20
    %33:f32 = sub %31, %32
    %34:f32 = mul %11, %21
    %35:f32 = mul %13, %19
    %36:f32 = sub %34, %35
    %37:f32 = mul %11, %20
    %38:f32 = mul %12, %19
    %39:f32 = sub %37, %38
    %40:f32 = mul %12, %17
    %41:f32 = mul %13, %16
    %42:f32 = sub %40, %41
    %43:f32 = mul %11, %17
    %44:f32 = mul %13, %15
    %45:f32 = sub %43, %44
    %46:f32 = mul %11, %16
    %47:f32 = mul %12, %15
    %48:f32 = sub %46, %47
    %49:f32 = mul %14, %21
    %50:f32 = mul %17, %18
    %51:f32 = sub %49, %50
    %52:f32 = mul %14, %20
    %53:f32 = mul %16, %18
    %54:f32 = sub %52, %53
    %55:f32 = mul %10, %21
    %56:f32 = mul %13, %18
    %57:f32 = sub %55, %56
    %58:f32 = mul %10, %20
    %59:f32 = mul %12, %18
    %60:f32 = sub %58, %59
    %61:f32 = mul %10, %17
    %62:f32 = mul %13, %14
    %63:f32 = sub %61, %62
    %64:f32 = mul %10, %16
    %65:f32 = mul %12, %14
    %66:f32 = sub %64, %65
    %67:f32 = mul %14, %19
    %68:f32 = mul %15, %18
    %69:f32 = sub %67, %68
    %70:f32 = mul %10, %19
    %71:f32 = mul %11, %18
    %72:f32 = sub %70, %71
    %73:f32 = mul %10, %15
    %74:f32 = mul %11, %14
    %75:f32 = sub %73, %74
    %76:f32 = negation %7
    %77:f32 = mul %11, %24
    %78:f32 = mul %12, %27
    %79:f32 = mul %13, %30
    %80:f32 = sub %77, %78
    %81:f32 = add %80, %79
    %82:f32 = mul %76, %24
    %83:f32 = mul %8, %27
    %84:f32 = mul %9, %30
    %85:f32 = add %82, %83
    %86:f32 = sub %85, %84
    %87:f32 = mul %7, %33
    %88:f32 = mul %8, %36
    %89:f32 = mul %9, %39
    %90:f32 = sub %87, %88
    %91:f32 = add %90, %89
    %92:f32 = mul %76, %42
    %93:f32 = mul %8, %45
    %94:f32 = mul %9, %48
    %95:f32 = add %92, %93
    %96:f32 = sub %95, %94
    %97:f32 = negation %10
    %98:f32 = negation %6
    %99:f32 = mul %97, %24
    %100:f32 = mul %12, %51
    %101:f32 = mul %13, %54
    %102:f32 = add %99, %100
    %103:f32 = sub %102, %101
    %104:f32 = mul %6, %24
    %105:f32 = mul %8, %51
    %106:f32 = mul %9, %54
    %107:f32 = sub %104, %105
    %108:f32 = add %107, %106
    %109:f32 = mul %98, %33
    %110:f32 = mul %8, %57
    %111:f32 = mul %9, %60
    %112:f32 = add %109, %110
    %113:f32 = sub %112, %111
    %114:f32 = mul %6, %42
    %115:f32 = mul %8, %63
    %116:f32 = mul %9, %66
    %117:f32 = sub %114, %115
    %118:f32 = add %117, %116
    %119:f32 = mul %10, %27
    %120:f32 = mul %11, %51
    %121:f32 = mul %13, %69
    %122:f32 = sub %119, %120
    %123:f32 = add %122, %121
    %124:f32 = mul %98, %27
    %125:f32 = mul %7, %51
    %126:f32 = mul %9, %69
    %127:f32 = add %124, %125
    %128:f32 = sub %127, %126
    %129:f32 = mul %6, %36
    %130:f32 = mul %7, %57
    %131:f32 = mul %9, %72
    %132:f32 = sub %129, %130
    %133:f32 = add %132, %131
    %134:f32 = mul %98, %45
    %135:f32 = mul %7, %63
    %136:f32 = mul %9, %75
    %137:f32 = add %134, %135
    %138:f32 = sub %137, %136
    %139:f32 = mul %97, %30
    %140:f32 = mul %11, %54
    %141:f32 = mul %12, %69
    %142:f32 = add %139, %140
    %143:f32 = sub %142, %141
    %144:f32 = mul %6, %30
    %145:f32 = mul %7, %54
    %146:f32 = mul %8, %69
    %147:f32 = sub %144, %145
    %148:f32 = add %147, %146
    %149:f32 = mul %98, %39
    %150:f32 = mul %7, %60
    %151:f32 = mul %8, %72
    %152:f32 = add %149, %150
    %153:f32 = sub %152, %151
    %154:f32 = mul %6, %48
    %155:f32 = mul %7, %66
    %156:f32 = mul %8, %75
    %157:f32 = sub %154, %155
    %158:f32 = add %157, %156
    %159:vec4<f32> = construct %81, %86, %91, %96
    %160:vec4<f32> = construct %103, %108, %113, %118
    %161:vec4<f32> = construct %123, %128, %133, %138
    %162:vec4<f32> = construct %143, %148, %153, %158
    %163:mat4x4<f32> = construct %159, %160, %161, %162
    %164:mat4x4<f32> = mul %5, %163
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Inverse_Mat4x4h) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* m = b.Let(
            "m", b.Construct(ty.mat4x4<f16>(), b.Splat(ty.vec4h(), 10_h), b.Splat(ty.vec4h(), 20_h),
                             b.Splat(ty.vec4h(), 30_h), b.Splat(ty.vec4h(), 40_h)));
        b.Call<spirv::ir::BuiltinCall>(ty.mat4x4<f16>(), spirv::BuiltinFn::kInverse, m);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat4x4<f16> = construct vec4<f16>(10.0h), vec4<f16>(20.0h), vec4<f16>(30.0h), vec4<f16>(40.0h)
    %m:mat4x4<f16> = let %2
    %4:mat4x4<f16> = spirv.inverse %m
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:mat4x4<f16> = construct vec4<f16>(10.0h), vec4<f16>(20.0h), vec4<f16>(30.0h), vec4<f16>(40.0h)
    %m:mat4x4<f16> = let %2
    %4:f16 = determinant %m
    %5:f16 = div 1.0h, %4
    %6:f16 = access %m, 0u, 0u
    %7:f16 = access %m, 0u, 1u
    %8:f16 = access %m, 0u, 2u
    %9:f16 = access %m, 0u, 3u
    %10:f16 = access %m, 1u, 0u
    %11:f16 = access %m, 1u, 1u
    %12:f16 = access %m, 1u, 2u
    %13:f16 = access %m, 1u, 3u
    %14:f16 = access %m, 2u, 0u
    %15:f16 = access %m, 2u, 1u
    %16:f16 = access %m, 2u, 2u
    %17:f16 = access %m, 2u, 3u
    %18:f16 = access %m, 3u, 0u
    %19:f16 = access %m, 3u, 1u
    %20:f16 = access %m, 3u, 2u
    %21:f16 = access %m, 3u, 3u
    %22:f16 = mul %16, %21
    %23:f16 = mul %17, %20
    %24:f16 = sub %22, %23
    %25:f16 = mul %15, %21
    %26:f16 = mul %17, %19
    %27:f16 = sub %25, %26
    %28:f16 = mul %15, %20
    %29:f16 = mul %16, %19
    %30:f16 = sub %28, %29
    %31:f16 = mul %12, %21
    %32:f16 = mul %13, %20
    %33:f16 = sub %31, %32
    %34:f16 = mul %11, %21
    %35:f16 = mul %13, %19
    %36:f16 = sub %34, %35
    %37:f16 = mul %11, %20
    %38:f16 = mul %12, %19
    %39:f16 = sub %37, %38
    %40:f16 = mul %12, %17
    %41:f16 = mul %13, %16
    %42:f16 = sub %40, %41
    %43:f16 = mul %11, %17
    %44:f16 = mul %13, %15
    %45:f16 = sub %43, %44
    %46:f16 = mul %11, %16
    %47:f16 = mul %12, %15
    %48:f16 = sub %46, %47
    %49:f16 = mul %14, %21
    %50:f16 = mul %17, %18
    %51:f16 = sub %49, %50
    %52:f16 = mul %14, %20
    %53:f16 = mul %16, %18
    %54:f16 = sub %52, %53
    %55:f16 = mul %10, %21
    %56:f16 = mul %13, %18
    %57:f16 = sub %55, %56
    %58:f16 = mul %10, %20
    %59:f16 = mul %12, %18
    %60:f16 = sub %58, %59
    %61:f16 = mul %10, %17
    %62:f16 = mul %13, %14
    %63:f16 = sub %61, %62
    %64:f16 = mul %10, %16
    %65:f16 = mul %12, %14
    %66:f16 = sub %64, %65
    %67:f16 = mul %14, %19
    %68:f16 = mul %15, %18
    %69:f16 = sub %67, %68
    %70:f16 = mul %10, %19
    %71:f16 = mul %11, %18
    %72:f16 = sub %70, %71
    %73:f16 = mul %10, %15
    %74:f16 = mul %11, %14
    %75:f16 = sub %73, %74
    %76:f16 = negation %7
    %77:f16 = mul %11, %24
    %78:f16 = mul %12, %27
    %79:f16 = mul %13, %30
    %80:f16 = sub %77, %78
    %81:f16 = add %80, %79
    %82:f16 = mul %76, %24
    %83:f16 = mul %8, %27
    %84:f16 = mul %9, %30
    %85:f16 = add %82, %83
    %86:f16 = sub %85, %84
    %87:f16 = mul %7, %33
    %88:f16 = mul %8, %36
    %89:f16 = mul %9, %39
    %90:f16 = sub %87, %88
    %91:f16 = add %90, %89
    %92:f16 = mul %76, %42
    %93:f16 = mul %8, %45
    %94:f16 = mul %9, %48
    %95:f16 = add %92, %93
    %96:f16 = sub %95, %94
    %97:f16 = negation %10
    %98:f16 = negation %6
    %99:f16 = mul %97, %24
    %100:f16 = mul %12, %51
    %101:f16 = mul %13, %54
    %102:f16 = add %99, %100
    %103:f16 = sub %102, %101
    %104:f16 = mul %6, %24
    %105:f16 = mul %8, %51
    %106:f16 = mul %9, %54
    %107:f16 = sub %104, %105
    %108:f16 = add %107, %106
    %109:f16 = mul %98, %33
    %110:f16 = mul %8, %57
    %111:f16 = mul %9, %60
    %112:f16 = add %109, %110
    %113:f16 = sub %112, %111
    %114:f16 = mul %6, %42
    %115:f16 = mul %8, %63
    %116:f16 = mul %9, %66
    %117:f16 = sub %114, %115
    %118:f16 = add %117, %116
    %119:f16 = mul %10, %27
    %120:f16 = mul %11, %51
    %121:f16 = mul %13, %69
    %122:f16 = sub %119, %120
    %123:f16 = add %122, %121
    %124:f16 = mul %98, %27
    %125:f16 = mul %7, %51
    %126:f16 = mul %9, %69
    %127:f16 = add %124, %125
    %128:f16 = sub %127, %126
    %129:f16 = mul %6, %36
    %130:f16 = mul %7, %57
    %131:f16 = mul %9, %72
    %132:f16 = sub %129, %130
    %133:f16 = add %132, %131
    %134:f16 = mul %98, %45
    %135:f16 = mul %7, %63
    %136:f16 = mul %9, %75
    %137:f16 = add %134, %135
    %138:f16 = sub %137, %136
    %139:f16 = mul %97, %30
    %140:f16 = mul %11, %54
    %141:f16 = mul %12, %69
    %142:f16 = add %139, %140
    %143:f16 = sub %142, %141
    %144:f16 = mul %6, %30
    %145:f16 = mul %7, %54
    %146:f16 = mul %8, %69
    %147:f16 = sub %144, %145
    %148:f16 = add %147, %146
    %149:f16 = mul %98, %39
    %150:f16 = mul %7, %60
    %151:f16 = mul %8, %72
    %152:f16 = add %149, %150
    %153:f16 = sub %152, %151
    %154:f16 = mul %6, %48
    %155:f16 = mul %7, %66
    %156:f16 = mul %8, %75
    %157:f16 = sub %154, %155
    %158:f16 = add %157, %156
    %159:vec4<f16> = construct %81, %86, %91, %96
    %160:vec4<f16> = construct %103, %108, %113, %118
    %161:vec4<f16> = construct %123, %128, %133, %138
    %162:vec4<f16> = construct %143, %148, %153, %158
    %163:mat4x4<f16> = construct %159, %160, %161, %162
    %164:mat4x4<f16> = mul %5, %163
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Scalar_UnsignedArg) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kSign,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               b.Constant(10_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.sign<i32> 10u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:i32 = sign %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Scalar_UnsignedResult) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kSign,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               b.Constant(10_i));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.sign<u32> 10i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = sign 10i
    %3:u32 = bitcast<u32> %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Scalar_UnsignedArgAndResult) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kSign,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               b.Constant(10_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.sign<u32> 10u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:i32 = sign %2
    %4:u32 = bitcast<u32> %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Scalar_SignedArgAndResult) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kSign,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               b.Constant(10_i));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.sign<i32> 10i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = sign 10i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Vector_UnsignedArg) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kSign,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               b.Splat(ty.vec2u(), (10_u)));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.sign<i32> vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %3:vec2<i32> = sign %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Vector_UnsignedResult) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kSign,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.sign<u32> vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = sign vec2<i32>(10i)
    %3:vec2<u32> = bitcast<vec2<u32>> %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Vector_UnsignedArgAndResult) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kSign,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.sign<u32> vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %3:vec2<i32> = sign %2
    %4:vec2<u32> = bitcast<vec2<u32>> %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SSign_Vector_SignedArgAndResult) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kSign,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.sign<i32> vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = sign vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

struct SpirvReaderParams {
    spirv::BuiltinFn fn;
    std::string spv_name;
    std::string wgsl_name;
};
[[maybe_unused]] inline std::ostream& operator<<(std::ostream& out, SpirvReaderParams c) {
    out << c.spv_name;
    return out;
}

using SpirvReader_BuiltinsTest_OneParamSigned =
    core::ir::transform::TransformTestWithParam<SpirvReaderParams>;

TEST_P(SpirvReader_BuiltinsTest_OneParamSigned, UnsignedToUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), params.fn,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10u
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:i32 = )" + params.wgsl_name +
                  R"( %2
    %4:u32 = bitcast<u32> %3
    %5:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %6:vec2<i32> = )" +
                  params.wgsl_name + R"( %5
    %7:vec2<u32> = bitcast<vec2<u32>> %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_OneParamSigned, UnsignedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), params.fn,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10u
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:i32 = )" + params.wgsl_name +
                  R"( %2
    %4:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %5:vec2<i32> = )" +
                  params.wgsl_name + R"( %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_OneParamSigned, SignedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), params.fn,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10i
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + params.wgsl_name +
                  R"( 10i
    %3:vec2<i32> = )" +
                  params.wgsl_name + R"( vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_OneParamSigned, SignedToUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), params.fn,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10i
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + params.wgsl_name +
                  R"( 10i
    %3:u32 = bitcast<u32> %2
    %4:vec2<i32> = )" +
                  params.wgsl_name + R"( vec2<i32>(10i)
    %5:vec2<u32> = bitcast<vec2<u32>> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReader_BuiltinsTest_OneParamSigned,
                         ::testing::Values(SpirvReaderParams{spirv::BuiltinFn::kAbs, "abs", "abs"},
                                           SpirvReaderParams{spirv::BuiltinFn::kFindSMsb,
                                                             "find_s_msb", "firstLeadingBit"}));

using SpirvReader_BuiltinsTest_OneParamUnsigned =
    core::ir::transform::TransformTestWithParam<SpirvReaderParams>;

TEST_P(SpirvReader_BuiltinsTest_OneParamUnsigned, UnsignedToUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), params.fn,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10u
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + params.wgsl_name +
                  R"( 10u
    %3:vec2<u32> = )" +
                  params.wgsl_name + R"( vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_OneParamUnsigned, UnsignedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), params.fn,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10u
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + params.wgsl_name +
                  R"( 10u
    %3:i32 = bitcast<i32> %2
    %4:vec2<u32> = )" +
                  params.wgsl_name + R"( vec2<u32>(10u)
    %5:vec2<i32> = bitcast<vec2<i32>> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_OneParamUnsigned, SignedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), params.fn,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10i
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = )" + params.wgsl_name +
                  R"( %2
    %4:i32 = bitcast<i32> %3
    %5:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %6:vec2<u32> = )" +
                  params.wgsl_name + R"( %5
    %7:vec2<i32> = bitcast<vec2<i32>> %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_OneParamUnsigned, SignedToUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), params.fn,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10i
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = )" + params.wgsl_name +
                  R"( %2
    %4:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %5:vec2<u32> = )" +
                  params.wgsl_name + R"( %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReader_BuiltinsTest_OneParamUnsigned,
                         ::testing::Values(SpirvReaderParams{spirv::BuiltinFn::kFindUMsb,
                                                             "find_u_msb", "firstLeadingBit"}));

using SpirvReader_BuiltinsTest_TwoParamSigned =
    core::ir::transform::TransformTestWithParam<SpirvReaderParams>;

TEST_P(SpirvReader_BuiltinsTest_TwoParamSigned, UnsignedToUnsigned) {
    auto& params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, 10_u, 15_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()},
            b.Splat(ty.vec2u(), 10_u), b.Splat(ty.vec2u(), 15_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10u, 15u
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<u32>(10u), vec2<u32>(15u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:i32 = bitcast<i32> 15u
    %4:i32 = )" + params.wgsl_name +
                  R"( %2, %3
    %5:u32 = bitcast<u32> %4
    %6:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %7:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(15u)
    %8:vec2<i32> = )" +
                  params.wgsl_name + R"( %6, %7
    %9:vec2<u32> = bitcast<vec2<u32>> %8
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_TwoParamSigned, SignedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, 10_i, 15_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()},
            b.Splat(ty.vec2i(), 10_i), b.Splat(ty.vec2i(), 15_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10i, 15i
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<i32>(10i), vec2<i32>(15i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = )" + params.wgsl_name +
                  R"( 10i, 15i
    %3:vec2<i32> = )" +
                  params.wgsl_name + R"( vec2<i32>(10i), vec2<i32>(15i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_TwoParamSigned, MixedToUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, 10_i, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()},
            b.Splat(ty.vec2i(), 10_i), b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10i, 10u
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<i32>(10i), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:i32 = )" + params.wgsl_name +
                  R"( 10i, %2
    %4:u32 = bitcast<u32> %3
    %5:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %6:vec2<i32> = )" +
                  params.wgsl_name + R"( vec2<i32>(10i), %5
    %7:vec2<u32> = bitcast<vec2<u32>> %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_TwoParamSigned, MixedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, 10_u, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()},
            b.Splat(ty.vec2u(), 10_u), b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10u, 10i
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<u32>(10u), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:i32 = )" + params.wgsl_name +
                  R"( %2, 10i
    %4:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %5:vec2<i32> = )" +
                  params.wgsl_name + R"( %4, vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReader_BuiltinsTest_TwoParamSigned,
    ::testing::Values(SpirvReaderParams{spirv::BuiltinFn::kSMax, "s_max", "max"},
                      SpirvReaderParams{spirv::BuiltinFn::kSMin, "s_min", "min"}));

using SpirvReader_BuiltinsTest_TwoParamUnsigned =
    core::ir::transform::TransformTestWithParam<SpirvReaderParams>;

TEST_P(SpirvReader_BuiltinsTest_TwoParamUnsigned, UnsignedToUnsigned) {
    auto& params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, 10_u, 15_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()},
            b.Splat(ty.vec2u(), 10_u), b.Splat(ty.vec2u(), 15_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10u, 15u
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<u32>(10u), vec2<u32>(15u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = )" + params.wgsl_name +
                  R"( 10u, 15u
    %3:vec2<u32> = )" +
                  params.wgsl_name + R"( vec2<u32>(10u), vec2<u32>(15u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_TwoParamUnsigned, SignedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, 10_i, 15_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()},
            b.Splat(ty.vec2i(), 10_i), b.Splat(ty.vec2i(), 15_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10i, 15i
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<i32>(10i), vec2<i32>(15i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = bitcast<u32> 15i
    %4:u32 = )" + params.wgsl_name +
                  R"( %2, %3
    %5:i32 = bitcast<i32> %4
    %6:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %7:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(15i)
    %8:vec2<u32> = )" +
                  params.wgsl_name + R"( %6, %7
    %9:vec2<i32> = bitcast<vec2<i32>> %8
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_TwoParamUnsigned, MixedToUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, 10_i, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()},
            b.Splat(ty.vec2i(), 10_i), b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.)" +
               params.spv_name + R"(<u32> 10i, 10u
    %3:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> vec2<i32>(10i), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = )" + params.wgsl_name +
                  R"( %2, 10u
    %4:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %5:vec2<u32> = )" +
                  params.wgsl_name + R"( %4, vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsTest_TwoParamUnsigned, MixedToSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, 10_u, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()},
            b.Splat(ty.vec2u(), 10_u), b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.)" +
               params.spv_name + R"(<i32> 10u, 10i
    %3:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> vec2<u32>(10u), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = )" + params.wgsl_name +
                  R"( 10u, %2
    %4:i32 = bitcast<i32> %3
    %5:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %6:vec2<u32> = )" +
                  params.wgsl_name + R"( vec2<u32>(10u), %5
    %7:vec2<i32> = bitcast<vec2<i32>> %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReader_BuiltinsTest_TwoParamUnsigned,
    ::testing::Values(SpirvReaderParams{spirv::BuiltinFn::kUMax, "u_max", "max"},
                      SpirvReaderParams{spirv::BuiltinFn::kUMin, "u_min", "min"}));

TEST_F(SpirvReader_BuiltinsTest, SClamp_UnsignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kSClamp,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               10_u, 15_u, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), spirv::BuiltinFn::kSClamp, Vector<core::ir::TemplateParameter, 1>{ty.u32()},
            b.Splat(ty.vec2u(), 10_u), b.Splat(ty.vec2u(), 15_u), b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.s_clamp<u32> 10u, 15u, 10u
    %3:vec2<u32> = spirv.s_clamp<u32> vec2<u32>(10u), vec2<u32>(15u), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:i32 = bitcast<i32> 15u
    %4:i32 = bitcast<i32> 10u
    %5:i32 = clamp %2, %3, %4
    %6:u32 = bitcast<u32> %5
    %7:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %8:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(15u)
    %9:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %10:vec2<i32> = clamp %7, %8, %9
    %11:vec2<u32> = bitcast<vec2<u32>> %10
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SClamp_SignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kSClamp,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               10_i, 15_i, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), spirv::BuiltinFn::kSClamp, Vector<core::ir::TemplateParameter, 1>{ty.i32()},
            b.Splat(ty.vec2i(), 10_i), b.Splat(ty.vec2i(), 15_i), b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.s_clamp<i32> 10i, 15i, 10i
    %3:vec2<i32> = spirv.s_clamp<i32> vec2<i32>(10i), vec2<i32>(15i), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = clamp 10i, 15i, 10i
    %3:vec2<i32> = clamp vec2<i32>(10i), vec2<i32>(15i), vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SClamp_MixedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kSClamp,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               10_i, 10_u, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), spirv::BuiltinFn::kSClamp, Vector<core::ir::TemplateParameter, 1>{ty.u32()},
            b.Splat(ty.vec2i(), 10_i), b.Splat(ty.vec2u(), 10_u), b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.s_clamp<u32> 10i, 10u, 10i
    %3:vec2<u32> = spirv.s_clamp<u32> vec2<i32>(10i), vec2<u32>(10u), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:i32 = clamp 10i, %2, 10i
    %4:u32 = bitcast<u32> %3
    %5:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %6:vec2<i32> = clamp vec2<i32>(10i), %5, vec2<i32>(10i)
    %7:vec2<u32> = bitcast<vec2<u32>> %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SClamp_MixedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kSClamp,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               10_u, 10_i, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), spirv::BuiltinFn::kSClamp, Vector<core::ir::TemplateParameter, 1>{ty.i32()},
            b.Splat(ty.vec2u(), 10_u), b.Splat(ty.vec2i(), 10_i), b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.s_clamp<i32> 10u, 10i, 10u
    %3:vec2<i32> = spirv.s_clamp<i32> vec2<u32>(10u), vec2<i32>(10i), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:i32 = bitcast<i32> 10u
    %4:i32 = clamp %2, 10i, %3
    %5:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %6:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %7:vec2<i32> = clamp %5, vec2<i32>(10i), %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, UClamp_UnsignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kUClamp,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               10_u, 15_u, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), spirv::BuiltinFn::kUClamp, Vector<core::ir::TemplateParameter, 1>{ty.u32()},
            b.Splat(ty.vec2u(), 10_u), b.Splat(ty.vec2u(), 15_u), b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.u_clamp<u32> 10u, 15u, 10u
    %3:vec2<u32> = spirv.u_clamp<u32> vec2<u32>(10u), vec2<u32>(15u), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = clamp 10u, 15u, 10u
    %3:vec2<u32> = clamp vec2<u32>(10u), vec2<u32>(15u), vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, UClamp_SignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kUClamp,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               10_i, 15_i, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), spirv::BuiltinFn::kUClamp, Vector<core::ir::TemplateParameter, 1>{ty.i32()},
            b.Splat(ty.vec2i(), 10_i), b.Splat(ty.vec2i(), 15_i), b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.u_clamp<i32> 10i, 15i, 10i
    %3:vec2<i32> = spirv.u_clamp<i32> vec2<i32>(10i), vec2<i32>(15i), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = bitcast<u32> 15i
    %4:u32 = bitcast<u32> 10i
    %5:u32 = clamp %2, %3, %4
    %6:i32 = bitcast<i32> %5
    %7:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %8:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(15i)
    %9:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %10:vec2<u32> = clamp %7, %8, %9
    %11:vec2<i32> = bitcast<vec2<i32>> %10
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, UClamp_MixedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kUClamp,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               10_i, 10_u, 10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), spirv::BuiltinFn::kUClamp, Vector<core::ir::TemplateParameter, 1>{ty.u32()},
            b.Splat(ty.vec2i(), 10_i), b.Splat(ty.vec2u(), 10_u), b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.u_clamp<u32> 10i, 10u, 10i
    %3:vec2<u32> = spirv.u_clamp<u32> vec2<i32>(10i), vec2<u32>(10u), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = bitcast<u32> 10i
    %4:u32 = clamp %2, 10u, %3
    %5:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %6:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %7:vec2<u32> = clamp %5, vec2<u32>(10u), %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, UClamp_MixedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kUClamp,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               10_u, 10_i, 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), spirv::BuiltinFn::kUClamp, Vector<core::ir::TemplateParameter, 1>{ty.i32()},
            b.Splat(ty.vec2u(), 10_u), b.Splat(ty.vec2i(), 10_i), b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.u_clamp<i32> 10u, 10i, 10u
    %3:vec2<i32> = spirv.u_clamp<i32> vec2<u32>(10u), vec2<i32>(10i), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = clamp 10u, %2, 10u
    %4:i32 = bitcast<i32> %3
    %5:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %6:vec2<u32> = clamp vec2<u32>(10u), %5, vec2<u32>(10u)
    %7:vec2<i32> = bitcast<vec2<i32>> %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FindILsb_SignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kFindILsb,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kFindILsb,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.find_i_lsb<i32> 10i
    %3:vec2<i32> = spirv.find_i_lsb<i32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = firstTrailingBit 10i
    %3:vec2<i32> = firstTrailingBit vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FindILsb_UnsignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kFindILsb,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kFindILsb,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.find_i_lsb<u32> 10u
    %3:vec2<u32> = spirv.find_i_lsb<u32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = firstTrailingBit 10u
    %3:vec2<u32> = firstTrailingBit vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FindILsb_SignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kFindILsb,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               10_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kFindILsb,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.find_i_lsb<u32> 10i
    %3:vec2<u32> = spirv.find_i_lsb<u32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = firstTrailingBit 10i
    %3:u32 = bitcast<u32> %2
    %4:vec2<i32> = firstTrailingBit vec2<i32>(10i)
    %5:vec2<u32> = bitcast<vec2<u32>> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FindILsb_UnsignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kFindILsb,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kFindILsb,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.find_i_lsb<i32> 10u
    %3:vec2<i32> = spirv.find_i_lsb<i32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = firstTrailingBit 10u
    %3:i32 = bitcast<i32> %2
    %4:vec2<u32> = firstTrailingBit vec2<u32>(10u)
    %5:vec2<i32> = bitcast<vec2<i32>> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Refract_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kRefract, 50_f, 60_f, 70_f);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.refract 50.0f, 60.0f, 70.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = construct 50.0f, 0.0f
    %3:vec2<f32> = construct 60.0f, 0.0f
    %4:vec2<f32> = refract %2, %3, 70.0f
    %5:f32 = swizzle %4, x
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Refract_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kRefract,
                                       b.Splat(ty.vec2f(), 10_f), b.Splat(ty.vec2f(), 20_f), 70_f);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.refract vec2<f32>(10.0f), vec2<f32>(20.0f), 70.0f
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = refract vec2<f32>(10.0f), vec2<f32>(20.0f), 70.0f
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FaceForward_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 50_f);
        auto* y = b.Let("y", 60_f);
        auto* z = b.Let("z", 70_f);
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kFaceForward, x, y, z);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:f32 = let 50.0f
    %y:f32 = let 60.0f
    %z:f32 = let 70.0f
    %5:f32 = spirv.face_forward %x, %y, %z
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:f32 = let 50.0f
    %y:f32 = let 60.0f
    %z:f32 = let 70.0f
    %5:f32 = negation %x
    %6:f32 = mul %y, %z
    %7:bool = lt %6, 0.0f
    %8:f32 = select %5, %x, %7
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FaceForward_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kFaceForward,
                                       b.Splat(ty.vec2f(), 10_f), b.Splat(ty.vec2f(), 20_f),
                                       b.Splat(ty.vec2f(), 30_f));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.face_forward vec2<f32>(10.0f), vec2<f32>(20.0f), vec2<f32>(30.0f)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = faceForward vec2<f32>(10.0f), vec2<f32>(20.0f), vec2<f32>(30.0f)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Reflect_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 50_f);
        auto* y = b.Let("y", 60_f);
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kReflect, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:f32 = let 50.0f
    %y:f32 = let 60.0f
    %4:f32 = spirv.reflect %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:f32 = let 50.0f
    %y:f32 = let 60.0f
    %4:f32 = mul %x, %y
    %5:f32 = mul %4, %y
    %6:f32 = mul %5, 2.0f
    %7:f32 = sub %x, %6
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Reflect_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kReflect,
                                       b.Splat(ty.vec2f(), 10_f), b.Splat(ty.vec2f(), 20_f));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.reflect vec2<f32>(10.0f), vec2<f32>(20.0f)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = reflect vec2<f32>(10.0f), vec2<f32>(20.0f)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Ldexp_ScalarSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kLdexp, 50_f, 10_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.ldexp 50.0f, 10i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = ldexp 50.0f, 10i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Ldexp_ScalarUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kLdexp, 50_f, 10_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.ldexp 50.0f, 10u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:f32 = ldexp 50.0f, %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Ldexp_VectorUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kLdexp,
                                       b.Splat(ty.vec2f(), 50_f), b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.ldexp vec2<f32>(50.0f), vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %3:vec2<f32> = ldexp vec2<f32>(50.0f), %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}
TEST_F(SpirvReader_BuiltinsTest, Ldexp_VectorSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kLdexp,
                                       b.Splat(ty.vec2f(), 50_f), b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.ldexp vec2<f32>(50.0f), vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = ldexp vec2<f32>(50.0f), vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Modf_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* v = b.Var(ty.ptr<function, f32>());
        auto* res = b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kModf, 50_f, v);
        b.Let(b.Multiply(res, res));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var undef
    %3:f32 = spirv.modf 50.0f, %2
    %4:f32 = mul %3, %3
    %5:f32 = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
__modf_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  whole:f32 @offset(4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, f32, read_write> = var undef
    %3:__modf_result_f32 = modf 50.0f
    %4:f32 = access %3, 1u
    store %2, %4
    %5:f32 = access %3, 0u
    %6:f32 = mul %5, %5
    %7:f32 = let %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Modf_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* v = b.Var(ty.ptr<function, vec2<f32>>());
        auto* res = b.Call<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kModf,
                                                   b.Splat(ty.vec2f(), 50_f), v);
        b.Let(b.Multiply(res, res));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<f32>, read_write> = var undef
    %3:vec2<f32> = spirv.modf vec2<f32>(50.0f), %2
    %4:vec2<f32> = mul %3, %3
    %5:vec2<f32> = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
__modf_result_vec2_f32 = struct @align(8) {
  fract:vec2<f32> @offset(0)
  whole:vec2<f32> @offset(8)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<f32>, read_write> = var undef
    %3:__modf_result_vec2_f32 = modf vec2<f32>(50.0f)
    %4:vec2<f32> = access %3, 1u
    store %2, %4
    %5:vec2<f32> = access %3, 0u
    %6:vec2<f32> = mul %5, %5
    %7:vec2<f32> = let %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Frexp_ScalarSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* v = b.Var(ty.ptr<function, i32>());
        auto* res = b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kFrexp, 50_f, v);
        b.Let(b.Multiply(res, res));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    %3:f32 = spirv.frexp 50.0f, %2
    %4:f32 = mul %3, %3
    %5:f32 = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, i32, read_write> = var undef
    %3:__frexp_result_f32 = frexp 50.0f
    %4:i32 = access %3, 1u
    store %2, %4
    %5:f32 = access %3, 0u
    %6:f32 = mul %5, %5
    %7:f32 = let %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Frexp_ScalarUnSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* v = b.Var(ty.ptr<function, u32>());
        auto* res = b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kFrexp, 50_f, v);
        b.Let(b.Multiply(res, res));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, u32, read_write> = var undef
    %3:f32 = spirv.frexp 50.0f, %2
    %4:f32 = mul %3, %3
    %5:f32 = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
__frexp_result_f32 = struct @align(4) {
  fract:f32 @offset(0)
  exp:i32 @offset(4)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, u32, read_write> = var undef
    %3:__frexp_result_f32 = frexp 50.0f
    %4:i32 = access %3, 1u
    %5:u32 = bitcast<u32> %4
    store %2, %5
    %6:f32 = access %3, 0u
    %7:f32 = mul %6, %6
    %8:f32 = let %7
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Frexp_VectorSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* v = b.Var(ty.ptr<function, vec2<i32>>());
        auto* res = b.Call<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kFrexp,
                                                   b.Splat(ty.vec2f(), 50_f), v);
        b.Let(b.Multiply(res, res));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<i32>, read_write> = var undef
    %3:vec2<f32> = spirv.frexp vec2<f32>(50.0f), %2
    %4:vec2<f32> = mul %3, %3
    %5:vec2<f32> = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
__frexp_result_vec2_f32 = struct @align(8) {
  fract:vec2<f32> @offset(0)
  exp:vec2<i32> @offset(8)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<i32>, read_write> = var undef
    %3:__frexp_result_vec2_f32 = frexp vec2<f32>(50.0f)
    %4:vec2<i32> = access %3, 1u
    store %2, %4
    %5:vec2<f32> = access %3, 0u
    %6:vec2<f32> = mul %5, %5
    %7:vec2<f32> = let %6
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Frexp_VectorUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* v = b.Var(ty.ptr<function, vec2<u32>>());
        auto* res = b.Call<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kFrexp,
                                                   b.Splat(ty.vec2f(), 50_f), v);
        b.Let(b.Multiply(res, res));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<u32>, read_write> = var undef
    %3:vec2<f32> = spirv.frexp vec2<f32>(50.0f), %2
    %4:vec2<f32> = mul %3, %3
    %5:vec2<f32> = let %4
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
__frexp_result_vec2_f32 = struct @align(8) {
  fract:vec2<f32> @offset(0)
  exp:vec2<i32> @offset(8)
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:ptr<function, vec2<u32>, read_write> = var undef
    %3:__frexp_result_vec2_f32 = frexp vec2<f32>(50.0f)
    %4:vec2<i32> = access %3, 1u
    %5:vec2<u32> = bitcast<vec2<u32>> %4
    store %2, %5
    %6:vec2<f32> = access %3, 0u
    %7:vec2<f32> = mul %6, %6
    %8:vec2<f32> = let %7
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Scalar_UnsignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitCount,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               10_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_count<u32> 10u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = countOneBits 10u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Scalar_UnsignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitCount,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               10_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_count<i32> 10u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = countOneBits 10u
    %3:i32 = bitcast<i32> %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Scalar_SignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitCount,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               10_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_count<u32> 10i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = countOneBits 10i
    %3:u32 = bitcast<u32> %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Scalar_SignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitCount,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               10_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_count<i32> 10i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = countOneBits 10i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Vector_UnsignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kBitCount,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_count<u32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = countOneBits vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Vector_UnsignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kBitCount,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               b.Splat(ty.vec2u(), 10_u));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_count<i32> vec2<u32>(10u)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = countOneBits vec2<u32>(10u)
    %3:vec2<i32> = bitcast<vec2<i32>> %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Vector_SignedToUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kBitCount,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_count<u32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = countOneBits vec2<i32>(10i)
    %3:vec2<u32> = bitcast<vec2<u32>> %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitCount_Vector_SignedToSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kBitCount,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               b.Splat(ty.vec2i(), 10_i));
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_count<i32> vec2<i32>(10i)
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = countOneBits vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_Int_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitFieldInsert, 10_i, 20_i,
                                       10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_insert 10i, 20i, 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = insertBits 10i, 20i, 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_Int_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitFieldInsert, 10_i, 20_i,
                                       10_i, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_insert 10i, 20i, 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = bitcast<u32> 20i
    %4:i32 = insertBits 10i, 20i, %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_IntVector_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kBitFieldInsert,
                                       b.Splat<vec2<i32>>(10_i), b.Splat<vec2<i32>>(20_i), 10_u,
                                       20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_insert vec2<i32>(10i), vec2<i32>(20i), 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = insertBits vec2<i32>(10i), vec2<i32>(20i), 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_IntVector_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kBitFieldInsert,
                                       b.Splat<vec2<i32>>(10_i), b.Splat<vec2<i32>>(20_i), 10_i,
                                       20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_insert vec2<i32>(10i), vec2<i32>(20i), 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = bitcast<u32> 20i
    %4:vec2<i32> = insertBits vec2<i32>(10i), vec2<i32>(20i), %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_Uint_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldInsert, 10_u, 20_u,
                                       10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_insert 10u, 20u, 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = insertBits 10u, 20u, 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_Uint_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldInsert, 10_u, 20_u,
                                       10_i, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_insert 10u, 20u, 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = bitcast<u32> 20i
    %4:u32 = insertBits 10u, 20u, %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_UintVector_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kBitFieldInsert,
                                       b.Splat<vec2<u32>>(10_u), b.Splat<vec2<u32>>(20_u), 10_u,
                                       20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_insert vec2<u32>(10u), vec2<u32>(20u), 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = insertBits vec2<u32>(10u), vec2<u32>(20u), 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_UintVector_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kBitFieldInsert,
                                       b.Splat<vec2<u32>>(10_u), b.Splat<vec2<u32>>(20_u), 10_i,
                                       20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_insert vec2<u32>(10u), vec2<u32>(20u), 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = bitcast<u32> 20i
    %4:vec2<u32> = insertBits vec2<u32>(10u), vec2<u32>(20u), %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldInsert_Uint_SignedOffsetAndUnsignedCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldInsert, 10_u, 20_u,
                                       10_i, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_insert 10u, 20u, 10i, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = insertBits 10u, 20u, %2, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_Int_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitFieldSExtract, 10_i, 10_u,
                                       20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_s_extract 10i, 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = extractBits 10i, 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_Int_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitFieldSExtract, 10_i, 10_i,
                                       20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_s_extract 10i, 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = bitcast<u32> 20i
    %4:i32 = extractBits 10i, %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_IntVector_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kBitFieldSExtract,
                                       b.Splat<vec2<i32>>(10_i), 10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_s_extract vec2<i32>(10i), 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = extractBits vec2<i32>(10i), 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_IntVector_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kBitFieldSExtract,
                                       b.Splat<vec2<i32>>(10_i), 10_i, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_s_extract vec2<i32>(10i), 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = bitcast<u32> 20i
    %4:vec2<i32> = extractBits vec2<i32>(10i), %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_IntVector_SignedOffsetAndUnsignedCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kBitFieldSExtract,
                                       b.Splat<vec2<i32>>(10_i), 10_i, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_s_extract vec2<i32>(10i), 10i, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:vec2<i32> = extractBits vec2<i32>(10i), %2, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_Uint_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldSExtract, 10_u, 10_u,
                                       20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_s_extract 10u, 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:i32 = extractBits %2, 10u, 20u
    %4:u32 = bitcast<u32> %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_Uint_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldSExtract, 10_u, 10_i,
                                       20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_s_extract 10u, 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:u32 = bitcast<u32> 10i
    %4:u32 = bitcast<u32> 20i
    %5:i32 = extractBits %2, %3, %4
    %6:u32 = bitcast<u32> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_UintVector_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kBitFieldSExtract,
                                       b.Splat<vec2<u32>>(10_u), 10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_s_extract vec2<u32>(10u), 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %3:vec2<i32> = extractBits %2, 10u, 20u
    %4:vec2<u32> = bitcast<vec2<u32>> %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_UintVector_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kBitFieldSExtract,
                                       b.Splat<vec2<u32>>(10_u), 10_i, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_s_extract vec2<u32>(10u), 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %3:u32 = bitcast<u32> 10i
    %4:u32 = bitcast<u32> 20i
    %5:vec2<i32> = extractBits %2, %3, %4
    %6:vec2<u32> = bitcast<vec2<u32>> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldSExtract_UintVector_SignedOffsetAndUnsignedCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kBitFieldSExtract,
                                       b.Splat<vec2<u32>>(10_u), 10_i, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_s_extract vec2<u32>(10u), 10i, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %3:u32 = bitcast<u32> 10i
    %4:vec2<i32> = extractBits %2, %3, 20u
    %5:vec2<u32> = bitcast<vec2<u32>> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_Uint_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldUExtract, 10_u, 10_u,
                                       20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_u_extract 10u, 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = extractBits 10u, 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_Uint_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kBitFieldUExtract, 10_u, 10_i,
                                       20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.bit_field_u_extract 10u, 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = bitcast<u32> 20i
    %4:u32 = extractBits 10u, %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_UintVector_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kBitFieldUExtract,
                                       b.Splat<vec2<u32>>(10_u), 10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_u_extract vec2<u32>(10u), 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = extractBits vec2<u32>(10u), 10u, 20u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_UintVector_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kBitFieldUExtract,
                                       b.Splat<vec2<u32>>(10_u), 10_i, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_u_extract vec2<u32>(10u), 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = bitcast<u32> 20i
    %4:vec2<u32> = extractBits vec2<u32>(10u), %2, %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_UintVector_UnsignedOffsetAndSignedCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kBitFieldUExtract,
                                       b.Splat<vec2<u32>>(10_u), 10_u, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.bit_field_u_extract vec2<u32>(10u), 10u, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 20i
    %3:vec2<u32> = extractBits vec2<u32>(10u), 10u, %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_Int_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitFieldUExtract, 10_i, 10_u,
                                       20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_u_extract 10i, 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = extractBits %2, 10u, 20u
    %4:i32 = bitcast<i32> %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_Int_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kBitFieldUExtract, 10_i, 10_i,
                                       20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.bit_field_u_extract 10i, 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:u32 = bitcast<u32> 10i
    %4:u32 = bitcast<u32> 20i
    %5:u32 = extractBits %2, %3, %4
    %6:i32 = bitcast<i32> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_IntVector_UnsignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kBitFieldUExtract,
                                       b.Splat<vec2<i32>>(10_i), 10_u, 20_u);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_u_extract vec2<i32>(10i), 10u, 20u
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %3:vec2<u32> = extractBits %2, 10u, 20u
    %4:vec2<i32> = bitcast<vec2<i32>> %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_IntVector_SignedOffsetAndCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kBitFieldUExtract,
                                       b.Splat<vec2<i32>>(10_i), 10_i, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_u_extract vec2<i32>(10i), 10i, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %3:u32 = bitcast<u32> 10i
    %4:u32 = bitcast<u32> 20i
    %5:vec2<u32> = extractBits %2, %3, %4
    %6:vec2<i32> = bitcast<vec2<i32>> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, BitFieldUExtract_IntVector_UnsignedOffsetAndSignedCount) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kBitFieldUExtract,
                                       b.Splat<vec2<i32>>(10_i), 10_u, 20_i);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.bit_field_u_extract vec2<i32>(10i), 10u, 20i
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %3:u32 = bitcast<u32> 20i
    %4:vec2<u32> = extractBits %2, 10u, %3
    %5:vec2<i32> = bitcast<vec2<i32>> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

struct BinaryCase {
    spirv::BuiltinFn fn;
    std::string ir;
};

using SpirvReader_BuiltinsMixedSignTest = core::ir::transform::TransformTestWithParam<BinaryCase>;

TEST_P(SpirvReader_BuiltinsMixedSignTest, Scalar_Signed_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 50_i);
        auto* y = b.Let("y", 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 50i
    %y:u32 = let 10u
    %4:i32 = spirv.)" +
               params.ir + R"(<i32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 50i
    %y:u32 = let 10u
    %4:i32 = bitcast<i32> %y
    %5:i32 = )" + params.ir +
                  R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Scalar_Signed_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 10_u);
        auto* y = b.Let("y", 50_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 10u
    %y:i32 = let 50i
    %4:i32 = spirv.)" +
               params.ir + R"(<i32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 10u
    %y:i32 = let 50i
    %4:u32 = bitcast<u32> %y
    %5:u32 = )" + params.ir +
                  R"( %x, %4
    %6:i32 = bitcast<i32> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Scalar_Signed_UnsignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 10_u);
        auto* y = b.Let("y", 20_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 10u
    %y:u32 = let 20u
    %4:i32 = spirv.)" +
               params.ir + R"(<i32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 10u
    %y:u32 = let 20u
    %4:u32 = )" + params.ir +
                  R"( %x, %y
    %5:i32 = bitcast<i32> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Scalar_Unsigned_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 50_i);
        auto* y = b.Let("y", 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 50i
    %y:u32 = let 10u
    %4:u32 = spirv.)" +
               params.ir + R"(<u32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 50i
    %y:u32 = let 10u
    %4:i32 = bitcast<i32> %y
    %5:i32 = )" + params.ir +
                  R"( %x, %4
    %6:u32 = bitcast<u32> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Scalar_Unsigned_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 10_u);
        auto* y = b.Let("y", 50_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 10u
    %y:i32 = let 50i
    %4:u32 = spirv.)" +
               params.ir + R"(<u32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 10u
    %y:i32 = let 50i
    %4:u32 = bitcast<u32> %y
    %5:u32 = )" + params.ir +
                  R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Scalar_Unsigned_SignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 50_i);
        auto* y = b.Let("y", 60_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 50i
    %y:i32 = let 60i
    %4:u32 = spirv.)" +
               params.ir + R"(<u32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 50i
    %y:i32 = let 60i
    %4:i32 = )" + params.ir +
                  R"( %x, %y
    %5:u32 = bitcast<u32> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Vector_Signed_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(50_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(10_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(50i)
    %y:vec2<u32> = let vec2<u32>(10u)
    %4:vec2<i32> = spirv.)" +
               params.ir + R"(<i32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(50i)
    %y:vec2<u32> = let vec2<u32>(10u)
    %4:vec2<i32> = bitcast<vec2<i32>> %y
    %5:vec2<i32> = )" +
                  params.ir +
                  R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Vector_Signed_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(10_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(50_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(10u)
    %y:vec2<i32> = let vec2<i32>(50i)
    %4:vec2<i32> = spirv.)" +
               params.ir + R"(<i32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(10u)
    %y:vec2<i32> = let vec2<i32>(50i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<u32> = )" +
                  params.ir +
                  R"( %x, %4
    %6:vec2<i32> = bitcast<vec2<i32>> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Vector_Signed_UnsignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(10_u));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(20_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(10u)
    %y:vec2<u32> = let vec2<u32>(20u)
    %4:vec2<i32> = spirv.)" +
               params.ir + R"(<i32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(10u)
    %y:vec2<u32> = let vec2<u32>(20u)
    %4:vec2<u32> = )" +
                  params.ir +
                  R"( %x, %y
    %5:vec2<i32> = bitcast<vec2<i32>> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Vector_Unsigned_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(50_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(10_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(50i)
    %y:vec2<u32> = let vec2<u32>(10u)
    %4:vec2<u32> = spirv.)" +
               params.ir + R"(<u32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(50i)
    %y:vec2<u32> = let vec2<u32>(10u)
    %4:vec2<i32> = bitcast<vec2<i32>> %y
    %5:vec2<i32> = )" +
                  params.ir +
                  R"( %x, %4
    %6:vec2<u32> = bitcast<vec2<u32>> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Vector_Unsigned_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(10_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(50_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(10u)
    %y:vec2<i32> = let vec2<i32>(50i)
    %4:vec2<u32> = spirv.)" +
               params.ir + R"(<u32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(10u)
    %y:vec2<i32> = let vec2<i32>(50i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<u32> = )" +
                  params.ir +
                  R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsMixedSignTest, Vector_Unsigned_SignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(50_i));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(60_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(50i)
    %y:vec2<i32> = let vec2<i32>(60i)
    %4:vec2<u32> = spirv.)" +
               params.ir + R"(<u32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(50i)
    %y:vec2<i32> = let vec2<i32>(60i)
    %4:vec2<i32> = )" +
                  params.ir +
                  R"( %x, %y
    %5:vec2<u32> = bitcast<vec2<u32>> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReader_BuiltinsMixedSignTest,
                         testing::Values(BinaryCase{spirv::BuiltinFn::kAdd, "add"},
                                         BinaryCase{spirv::BuiltinFn::kSub, "sub"},
                                         BinaryCase{spirv::BuiltinFn::kMul, "mul"}));

struct SignedBinaryCase {
    spirv::BuiltinFn fn;
    std::string ir;
    std::string wgsl;
};

using SpirvReader_BuiltinsSignedTest =
    core::ir::transform::TransformTestWithParam<SignedBinaryCase>;

TEST_P(SpirvReader_BuiltinsSignedTest, Scalar_Signed_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 50_i);
        auto* y = b.Let("y", 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 50i
    %y:u32 = let 10u
    %4:i32 = spirv.)" +
               params.ir + R"(<i32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 50i
    %y:u32 = let 10u
    %4:i32 = bitcast<i32> %y
    %5:i32 = )" + params.wgsl +
                  R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Scalar_Signed_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 10_u);
        auto* y = b.Let("y", 50_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 10u
    %y:i32 = let 50i
    %4:i32 = spirv.)" +
               params.ir + R"(<i32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 10u
    %y:i32 = let 50i
    %4:i32 = bitcast<i32> %x
    %5:i32 = )" + params.wgsl +
                  R"( %4, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Scalar_Signed_UnsignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 10_u);
        auto* y = b.Let("y", 20_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 10u
    %y:u32 = let 20u
    %4:i32 = spirv.)" +
               params.ir + R"(<i32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 10u
    %y:u32 = let 20u
    %4:i32 = bitcast<i32> %x
    %5:i32 = bitcast<i32> %y
    %6:i32 = )" + params.wgsl +
                  R"( %4, %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Scalar_Unsigned_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 50_i);
        auto* y = b.Let("y", 10_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 50i
    %y:u32 = let 10u
    %4:u32 = spirv.)" +
               params.ir + R"(<u32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 50i
    %y:u32 = let 10u
    %4:i32 = bitcast<i32> %y
    %5:i32 = )" + params.wgsl +
                  R"( %x, %4
    %6:u32 = bitcast<u32> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Scalar_Unsigned_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 10_u);
        auto* y = b.Let("y", 50_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 10u
    %y:i32 = let 50i
    %4:u32 = spirv.)" +
               params.ir + R"(<u32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 10u
    %y:i32 = let 50i
    %4:i32 = bitcast<i32> %x
    %5:i32 = )" + params.wgsl +
                  R"( %4, %y
    %6:u32 = bitcast<u32> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Scalar_Unsigned_SignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 50_i);
        auto* y = b.Let("y", 60_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 50i
    %y:i32 = let 60i
    %4:u32 = spirv.)" +
               params.ir + R"(<u32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 50i
    %y:i32 = let 60i
    %4:i32 = )" + params.wgsl +
                  R"( %x, %y
    %5:u32 = bitcast<u32> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Vector_Signed_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(50_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(10_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(50i)
    %y:vec2<u32> = let vec2<u32>(10u)
    %4:vec2<i32> = spirv.)" +
               params.ir + R"(<i32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(50i)
    %y:vec2<u32> = let vec2<u32>(10u)
    %4:vec2<i32> = bitcast<vec2<i32>> %y
    %5:vec2<i32> = )" +
                  params.wgsl +
                  R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Vector_Signed_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(10_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(50_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(10u)
    %y:vec2<i32> = let vec2<i32>(50i)
    %4:vec2<i32> = spirv.)" +
               params.ir + R"(<i32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(10u)
    %y:vec2<i32> = let vec2<i32>(50i)
    %4:vec2<i32> = bitcast<vec2<i32>> %x
    %5:vec2<i32> = )" +
                  params.wgsl +
                  R"( %4, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Vector_Signed_UnsignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(10_u));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(20_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(10u)
    %y:vec2<u32> = let vec2<u32>(20u)
    %4:vec2<i32> = spirv.)" +
               params.ir + R"(<i32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(10u)
    %y:vec2<u32> = let vec2<u32>(20u)
    %4:vec2<i32> = bitcast<vec2<i32>> %x
    %5:vec2<i32> = bitcast<vec2<i32>> %y
    %6:vec2<i32> = )" +
                  params.wgsl +
                  R"( %4, %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Vector_Unsigned_SignedUnsigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(50_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(10_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(50i)
    %y:vec2<u32> = let vec2<u32>(10u)
    %4:vec2<u32> = spirv.)" +
               params.ir + R"(<u32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(50i)
    %y:vec2<u32> = let vec2<u32>(10u)
    %4:vec2<i32> = bitcast<vec2<i32>> %y
    %5:vec2<i32> = )" +
                  params.wgsl +
                  R"( %x, %4
    %6:vec2<u32> = bitcast<vec2<u32>> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Vector_Unsigned_UnsignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(10_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(50_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(10u)
    %y:vec2<i32> = let vec2<i32>(50i)
    %4:vec2<u32> = spirv.)" +
               params.ir + R"(<u32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(10u)
    %y:vec2<i32> = let vec2<i32>(50i)
    %4:vec2<i32> = bitcast<vec2<i32>> %x
    %5:vec2<i32> = )" +
                  params.wgsl +
                  R"( %4, %y
    %6:vec2<u32> = bitcast<vec2<u32>> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BuiltinsSignedTest, Vector_Unsigned_SignedSigned) {
    auto params = GetParam();

    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(50_i));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(60_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(50i)
    %y:vec2<i32> = let vec2<i32>(60i)
    %4:vec2<u32> = spirv.)" +
               params.ir + R"(<u32> %x, %y
    ret
  }
}
)";

    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(50i)
    %y:vec2<i32> = let vec2<i32>(60i)
    %4:vec2<i32> = )" +
                  params.wgsl +
                  R"( %x, %y
    %5:vec2<u32> = bitcast<vec2<u32>> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

INSTANTIATE_TEST_SUITE_P(SpirvReader,
                         SpirvReader_BuiltinsSignedTest,
                         testing::Values(SignedBinaryCase{spirv::BuiltinFn::kSDiv, "s_div", "div"},
                                         SignedBinaryCase{spirv::BuiltinFn::kSMod, "s_mod",
                                                          "mod"}));

TEST_F(SpirvReader_BuiltinsTest, ConvertFToS_ScalarSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kConvertFToS,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               10_f);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.convert_f_to_s<i32> 10.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = convert 10.0f
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertFToS_ScalarUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kConvertFToS,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               10_f);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.convert_f_to_s<u32> 10.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = convert 10.0f
    %3:u32 = bitcast<u32> %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertFToS_VectorSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kConvertFToS,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()},
                                               b.Splat<vec2<f32>>(10_f));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = spirv.convert_f_to_s<i32> vec2<f32>(10.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = convert vec2<f32>(10.0f)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertFToS_VectorUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kConvertFToS,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()},
                                               b.Splat<vec2<f32>>(10_f));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = spirv.convert_f_to_s<u32> vec2<f32>(10.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = convert vec2<f32>(10.0f)
    %3:vec2<u32> = bitcast<vec2<u32>> %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertSToF_ScalarSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kConvertSToF,
                                               Vector<core::ir::TemplateParameter, 1>{ty.f32()},
                                               10_i);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.convert_s_to_f<f32> 10i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = convert 10i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertSToF_ScalarUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kConvertSToF,
                                               Vector<core::ir::TemplateParameter, 1>{ty.f32()},
                                               10_u);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.convert_s_to_f<f32> 10u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = bitcast<i32> 10u
    %3:f32 = convert %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertSToF_VectorSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kConvertSToF,
                                               Vector<core::ir::TemplateParameter, 1>{ty.f32()},
                                               b.Splat<vec2<i32>>(10_i));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.convert_s_to_f<f32> vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = convert vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertSToF_VectorUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kConvertSToF,
                                               Vector<core::ir::TemplateParameter, 1>{ty.f32()},
                                               b.Splat<vec2<u32>>(10_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.convert_s_to_f<f32> vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<i32> = bitcast<vec2<i32>> vec2<u32>(10u)
    %3:vec2<f32> = convert %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertUToF_ScalarSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kConvertUToF,
                                               Vector<core::ir::TemplateParameter, 1>{ty.f32()},
                                               10_i);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.convert_u_to_f<f32> 10i
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = bitcast<u32> 10i
    %3:f32 = convert %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertUToF_ScalarUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kConvertUToF,
                                               Vector<core::ir::TemplateParameter, 1>{ty.f32()},
                                               10_u);
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.convert_u_to_f<f32> 10u
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = convert 10u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertUToF_VectorSigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kConvertUToF,
                                               Vector<core::ir::TemplateParameter, 1>{ty.f32()},
                                               b.Splat<vec2<i32>>(10_i));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.convert_u_to_f<f32> vec2<i32>(10i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<u32> = bitcast<vec2<u32>> vec2<i32>(10i)
    %3:vec2<f32> = convert %2
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ConvertUToF_VectorUnsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kConvertUToF,
                                               Vector<core::ir::TemplateParameter, 1>{ty.f32()},
                                               b.Splat<vec2<u32>>(10_u));
        b.Return(ep);
    });

    auto* src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.convert_u_to_f<f32> vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto* expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = convert vec2<u32>(10u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

using SpirvReader_BitwiseTest = core::ir::transform::TransformTestWithParam<SpirvReaderParams>;

TEST_P(SpirvReader_BitwiseTest, Scalar_SignedSigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:i32 = spirv.)" +
               params.spv_name + R"(<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:i32 = )" + params.wgsl_name +
                  R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_SignedUnsigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 8_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 8u
    %4:i32 = spirv.)" +
               params.spv_name + R"(<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 8u
    %4:i32 = bitcast<i32> %y
    %5:i32 = )" + params.wgsl_name +
                  R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_UnsignedSigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 1_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:i32 = spirv.)" +
               params.spv_name + R"(<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:u32 = bitcast<u32> %y
    %5:u32 = )" + params.wgsl_name +
                  R"( %x, %4
    %6:i32 = bitcast<i32> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_UnsignedUnsigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:i32 = spirv.)" +
               params.spv_name + R"(<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:u32 = )" + params.wgsl_name +
                  R"( %x, %y
    %5:i32 = bitcast<i32> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_UnsignedUnsigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:u32 = spirv.)" +
               params.spv_name + R"(<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:u32 = )" + params.wgsl_name +
                  R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_UnsignedSigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 1_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:u32 = spirv.)" +
               params.spv_name + R"(<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:u32 = bitcast<u32> %y
    %5:u32 = )" + params.wgsl_name +
                  R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_SignedUnsigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 8_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 8u
    %4:u32 = spirv.)" +
               params.spv_name + R"(<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 8u
    %4:i32 = bitcast<i32> %y
    %5:i32 = )" + params.wgsl_name +
                  R"( %x, %4
    %6:u32 = bitcast<u32> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Scalar_SignedSigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.u32(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:u32 = spirv.)" +
               params.spv_name + R"(<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:i32 = )" + params.wgsl_name +
                  R"( %x, %y
    %5:u32 = bitcast<u32> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_SignedSigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(2_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<i32> = )" +
                  params.wgsl_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_SignedUnsigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(8_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(8u)
    %4:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(8u)
    %4:vec2<i32> = bitcast<vec2<i32>> %y
    %5:vec2<i32> = )" +
                  params.wgsl_name + R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_UnsignedSigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(1_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<u32> = )" +
                  params.wgsl_name + R"( %x, %4
    %6:vec2<i32> = bitcast<vec2<i32>> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_UnsignedUnsigned_Signed) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2i(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = spirv.)" +
               params.spv_name + R"(<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = )" +
                  params.wgsl_name + R"( %x, %y
    %5:vec2<i32> = bitcast<vec2<i32>> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_UnsignedUnsigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = )" +
                  params.wgsl_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_UnsignedSigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(1_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<u32> = )" +
                  params.wgsl_name + R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_SignedUnsigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(8_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(8u)
    %4:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(8u)
    %4:vec2<i32> = bitcast<vec2<i32>> %y
    %5:vec2<i32> = )" +
                  params.wgsl_name + R"( %x, %4
    %6:vec2<u32> = bitcast<vec2<u32>> %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_BitwiseTest, Vector_SignedSigned_Unsigned) {
    auto& params = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(2_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.vec2u(), params.fn, Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<u32> = spirv.)" +
               params.spv_name + R"(<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<i32> = )" +
                  params.wgsl_name + R"( %x, %y
    %5:vec2<u32> = bitcast<vec2<u32>> %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}
INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReader_BitwiseTest,
    ::testing::Values(SpirvReaderParams{spirv::BuiltinFn::kBitwiseAnd, "bitwise_and", "and"},
                      SpirvReaderParams{spirv::BuiltinFn::kBitwiseOr, "bitwise_or", "or"},
                      SpirvReaderParams{spirv::BuiltinFn::kBitwiseXor, "bitwise_xor", "xor"}));

using SpirvReader_IntegerTest = core::ir::transform::TransformTestWithParam<SpirvReaderParams>;
TEST_P(SpirvReader_IntegerTest, Scalar_SignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:bool = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:bool = )" + param.wgsl_name +
                  R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Scalar_SignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 8_u);
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 8u
    %4:bool = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 8u
    %4:i32 = bitcast<i32> %y
    %5:bool = )" + param.wgsl_name +
                  R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Scalar_UnsignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:bool = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:u32 = bitcast<u32> %y
    %5:bool = )" + param.wgsl_name +
                  R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Scalar_UnsignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 9_u);
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:bool = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:bool = )" + param.wgsl_name +
                  R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Vector_SignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(2_i));
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<bool> = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<bool> = )" +
                  param.wgsl_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Vector_SignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(8_u));
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(8u)
    %4:vec2<bool> = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(8u)
    %4:vec2<i32> = bitcast<vec2<i32>> %y
    %5:vec2<bool> = )" +
                  param.wgsl_name + R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Vector_UnsignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(1_i));
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<bool> = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<bool> = )" +
                  param.wgsl_name + R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_IntegerTest, Vector_UnsignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<bool> = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<bool> = )" +
                  param.wgsl_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}
INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReader_IntegerTest,
    ::testing::Values(SpirvReaderParams{spirv::BuiltinFn::kEqual, "equal", "eq"},
                      SpirvReaderParams{spirv::BuiltinFn::kNotEqual, "not_equal", "neq"}));

using SpirvReader_SignedIntegerTest =
    core::ir::transform::TransformTestWithParam<SpirvReaderParams>;
TEST_P(SpirvReader_SignedIntegerTest, Scalar_SignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:bool = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:bool = )" + param.wgsl_name +
                  R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Scalar_SignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 8_u);
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 8u
    %4:bool = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 8u
    %4:i32 = bitcast<i32> %y
    %5:bool = )" + param.wgsl_name +
                  R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Scalar_UnsignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:bool = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:i32 = bitcast<i32> %x
    %5:bool = )" + param.wgsl_name +
                  R"( %4, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Scalar_UnsignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 9_u);
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:bool = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:i32 = bitcast<i32> %x
    %5:i32 = bitcast<i32> %y
    %6:bool = )" + param.wgsl_name +
                  R"( %4, %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Vector_SignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(2_i));
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<bool> = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<bool> = )" +
                  param.wgsl_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Vector_SignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(8_u));
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(8u)
    %4:vec2<bool> = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(8u)
    %4:vec2<i32> = bitcast<vec2<i32>> %y
    %5:vec2<bool> = )" +
                  param.wgsl_name + R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Vector_UnsignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(1_i));
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<bool> = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<i32> = bitcast<vec2<i32>> %x
    %5:vec2<bool> = )" +
                  param.wgsl_name + R"( %4, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_SignedIntegerTest, Vector_UnsignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<bool> = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = bitcast<vec2<i32>> %x
    %5:vec2<i32> = bitcast<vec2<i32>> %y
    %6:vec2<bool> = )" +
                  param.wgsl_name + R"( %4, %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}
INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReader_SignedIntegerTest,
    ::testing::Values(
        SpirvReaderParams{spirv::BuiltinFn::kSGreaterThan, "s_greater_than", "gt"},
        SpirvReaderParams{spirv::BuiltinFn::kSGreaterThanEqual, "s_greater_than_equal", "gte"},
        SpirvReaderParams{spirv::BuiltinFn::kSLessThan, "s_less_than", "lt"},
        SpirvReaderParams{spirv::BuiltinFn::kSLessThanEqual, "s_less_than_equal", "lte"}));

using SpirvReader_UnsignedIntegerTest =
    core::ir::transform::TransformTestWithParam<SpirvReaderParams>;
TEST_P(SpirvReader_UnsignedIntegerTest, Scalar_SignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:bool = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:u32 = bitcast<u32> %x
    %5:u32 = bitcast<u32> %y
    %6:bool = )" + param.wgsl_name +
                  R"( %4, %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Scalar_SignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 8_u);
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 8u
    %4:bool = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 8u
    %4:u32 = bitcast<u32> %x
    %5:bool = )" + param.wgsl_name +
                  R"( %4, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Scalar_UnsignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 1_i);
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:bool = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:u32 = bitcast<u32> %y
    %5:bool = )" + param.wgsl_name +
                  R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Scalar_UnsignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 9_u);
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:bool = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:bool = )" + param.wgsl_name +
                  R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Vector_SignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(2_i));
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<bool> = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<u32> = bitcast<vec2<u32>> %x
    %5:vec2<u32> = bitcast<vec2<u32>> %y
    %6:vec2<bool> = )" +
                  param.wgsl_name + R"( %4, %5
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Vector_SignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(8_u));
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(8u)
    %4:vec2<bool> = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(8u)
    %4:vec2<u32> = bitcast<vec2<u32>> %x
    %5:vec2<bool> = )" +
                  param.wgsl_name + R"( %4, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Vector_UnsignedSigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(1_i));
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<bool> = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<bool> = )" +
                  param.wgsl_name + R"( %x, %4
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_P(SpirvReader_UnsignedIntegerTest, Vector_UnsignedUnsigned) {
    auto param = GetParam();
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.Call<spirv::ir::BuiltinCall>(ty.vec2<bool>(), param.fn, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<bool> = spirv.)" +
               param.spv_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<bool> = )" +
                  param.wgsl_name + R"( %x, %y
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}
INSTANTIATE_TEST_SUITE_P(
    SpirvReader,
    SpirvReader_UnsignedIntegerTest,
    ::testing::Values(
        SpirvReaderParams{spirv::BuiltinFn::kUGreaterThan, "u_greater_than", "gt"},
        SpirvReaderParams{spirv::BuiltinFn::kUGreaterThanEqual, "u_greater_than_equal", "gte"},
        SpirvReaderParams{spirv::BuiltinFn::kULessThan, "u_less_than", "lt"},
        SpirvReaderParams{spirv::BuiltinFn::kULessThanEqual, "u_less_than_equal", "lte"}));

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_UnsignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:u32 = spirv.shift_left_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:u32 = shl %x, %y
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_UnsignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 1_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:u32 = spirv.shift_left_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:u32 = bitcast<u32> %y
    %5:u32 = shl %x, %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_SignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 9u
    %4:u32 = spirv.shift_left_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 9u
    %4:i32 = shl %x, %y
    %5:u32 = bitcast<u32> %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_SignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:u32 = spirv.shift_left_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:u32 = bitcast<u32> %y
    %5:i32 = shl %x, %4
    %6:u32 = bitcast<u32> %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_UnsignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:i32 = spirv.shift_left_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:u32 = shl %x, %y
    %5:i32 = bitcast<i32> %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_UnsignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 1_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:i32 = spirv.shift_left_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:u32 = bitcast<u32> %y
    %5:u32 = shl %x, %4
    %6:i32 = bitcast<i32> %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_SignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 9u
    %4:i32 = spirv.shift_left_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 9u
    %4:i32 = shl %x, %y
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Scalar_SignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:i32 = spirv.shift_left_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:u32 = bitcast<u32> %y
    %5:i32 = shl %x, %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_UnsignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = spirv.shift_left_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = shl %x, %y
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_UnsignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(1_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<u32> = spirv.shift_left_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<u32> = shl %x, %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_SignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = spirv.shift_left_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = shl %x, %y
    %5:vec2<u32> = bitcast<vec2<u32>> %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_SignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(2_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<u32> = spirv.shift_left_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<i32> = shl %x, %4
    %6:vec2<u32> = bitcast<vec2<u32>> %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_UnsignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = spirv.shift_left_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = shl %x, %y
    %5:vec2<i32> = bitcast<vec2<i32>> %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_UnsignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(1_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<i32> = spirv.shift_left_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<u32> = shl %x, %4
    %6:vec2<i32> = bitcast<vec2<i32>> %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_SignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = spirv.shift_left_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = shl %x, %y
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftLeftLogical_Vector_SignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(2_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kShiftLeftLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<i32> = spirv.shift_left_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<i32> = shl %x, %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_UnsignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:u32 = spirv.shift_right_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:u32 = shr %x, %y
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_UnsignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 1_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:u32 = spirv.shift_right_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:u32 = bitcast<u32> %y
    %5:u32 = shr %x, %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_SignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 9u
    %4:u32 = spirv.shift_right_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 9u
    %4:u32 = bitcast<u32> %x
    %5:u32 = shr %4, %y
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_SignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:u32 = spirv.shift_right_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:u32 = bitcast<u32> %x
    %5:u32 = bitcast<u32> %y
    %6:u32 = shr %4, %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_UnsignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:i32 = spirv.shift_right_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:u32 = shr %x, %y
    %5:i32 = bitcast<i32> %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_UnsignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 1_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:i32 = spirv.shift_right_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:u32 = bitcast<u32> %y
    %5:u32 = shr %x, %4
    %6:i32 = bitcast<i32> %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_SignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 9u
    %4:i32 = spirv.shift_right_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 9u
    %4:u32 = bitcast<u32> %x
    %5:u32 = shr %4, %y
    %6:i32 = bitcast<i32> %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Scalar_SignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:i32 = spirv.shift_right_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:u32 = bitcast<u32> %x
    %5:u32 = bitcast<u32> %y
    %6:u32 = shr %4, %5
    %7:i32 = bitcast<i32> %6
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_UnsignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = spirv.shift_right_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = shr %x, %y
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_UnsignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(1_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<u32> = spirv.shift_right_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<u32> = shr %x, %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_SignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = spirv.shift_right_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = bitcast<vec2<u32>> %x
    %5:vec2<u32> = shr %4, %y
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_SignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(2_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<u32> = spirv.shift_right_logical<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<u32> = bitcast<vec2<u32>> %x
    %5:vec2<u32> = bitcast<vec2<u32>> %y
    %6:vec2<u32> = shr %4, %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_UnsignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = spirv.shift_right_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = shr %x, %y
    %5:vec2<i32> = bitcast<vec2<i32>> %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_UnsignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(1_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<i32> = spirv.shift_right_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<u32> = shr %x, %4
    %6:vec2<i32> = bitcast<vec2<i32>> %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_SignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = spirv.shift_right_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = bitcast<vec2<u32>> %x
    %5:vec2<u32> = shr %4, %y
    %6:vec2<i32> = bitcast<vec2<i32>> %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightLogical_Vector_SignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(2_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kShiftRightLogical,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<i32> = spirv.shift_right_logical<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<u32> = bitcast<vec2<u32>> %x
    %5:vec2<u32> = bitcast<vec2<u32>> %y
    %6:vec2<u32> = shr %4, %5
    %7:vec2<i32> = bitcast<vec2<i32>> %6
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_UnsignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:u32 = spirv.shift_right_arithmetic<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:i32 = bitcast<i32> %x
    %5:i32 = shr %4, %y
    %6:u32 = bitcast<u32> %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_UnsignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 1_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:u32 = spirv.shift_right_arithmetic<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:i32 = bitcast<i32> %x
    %5:u32 = bitcast<u32> %y
    %6:i32 = shr %4, %5
    %7:u32 = bitcast<u32> %6
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_SignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 9u
    %4:u32 = spirv.shift_right_arithmetic<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 9u
    %4:i32 = shr %x, %y
    %5:u32 = bitcast<u32> %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_SignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:u32 = spirv.shift_right_arithmetic<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:u32 = bitcast<u32> %y
    %5:i32 = shr %x, %4
    %6:u32 = bitcast<u32> %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_UnsignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:i32 = spirv.shift_right_arithmetic<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:u32 = let 9u
    %4:i32 = bitcast<i32> %x
    %5:i32 = shr %4, %y
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_UnsignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 8_u);
        auto* y = b.Let("y", 1_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:i32 = spirv.shift_right_arithmetic<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:u32 = let 8u
    %y:i32 = let 1i
    %4:i32 = bitcast<i32> %x
    %5:u32 = bitcast<u32> %y
    %6:i32 = shr %4, %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_SignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 9_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 9u
    %4:i32 = spirv.shift_right_arithmetic<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:u32 = let 9u
    %4:i32 = shr %x, %y
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Scalar_SignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_i);
        auto* y = b.Let("y", 2_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:i32 = spirv.shift_right_arithmetic<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:i32 = let 1i
    %y:i32 = let 2i
    %4:u32 = bitcast<u32> %y
    %5:i32 = shr %x, %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_UnsignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = spirv.shift_right_arithmetic<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = bitcast<vec2<i32>> %x
    %5:vec2<i32> = shr %4, %y
    %6:vec2<u32> = bitcast<vec2<u32>> %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_UnsignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(1_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<u32> = spirv.shift_right_arithmetic<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<i32> = bitcast<vec2<i32>> %x
    %5:vec2<u32> = bitcast<vec2<u32>> %y
    %6:vec2<i32> = shr %4, %5
    %7:vec2<u32> = bitcast<vec2<u32>> %6
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_SignedUnsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<u32> = spirv.shift_right_arithmetic<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = shr %x, %y
    %5:vec2<u32> = bitcast<vec2<u32>> %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_SignedSigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(2_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<u32> = spirv.shift_right_arithmetic<u32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<i32> = shr %x, %4
    %6:vec2<u32> = bitcast<vec2<u32>> %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_UnsignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = spirv.shift_right_arithmetic<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = bitcast<vec2<i32>> %x
    %5:vec2<i32> = shr %4, %y
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_UnsignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<u32>>(8_u));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(1_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<i32> = spirv.shift_right_arithmetic<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<u32> = let vec2<u32>(8u)
    %y:vec2<i32> = let vec2<i32>(1i)
    %4:vec2<i32> = bitcast<vec2<i32>> %x
    %5:vec2<u32> = bitcast<vec2<u32>> %y
    %6:vec2<i32> = shr %4, %5
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_SignedUnsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<u32>>(9_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = spirv.shift_right_arithmetic<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<u32> = let vec2<u32>(9u)
    %4:vec2<i32> = shr %x, %y
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, ShiftRightArithmetic_Vector_SignedSigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<i32>>(1_i));
        auto* y = b.Let("y", b.Splat<vec2<i32>>(2_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kShiftRightArithmetic,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, x,
                                               y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<i32> = spirv.shift_right_arithmetic<i32> %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<i32> = let vec2<i32>(1i)
    %y:vec2<i32> = let vec2<i32>(2i)
    %4:vec2<u32> = bitcast<vec2<u32>> %y
    %5:vec2<i32> = shr %x, %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SpecConstantOp_Not) {
    auto* ep = b.ComputeFunction("foo");

    mod.properties.Add(core::ir::Property::kAllowOverrides);

    b.Append(b.ir.root_block, [&] {
        auto* comp = b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), spirv::BuiltinFn::kNot, Vector<core::ir::TemplateParameter, 1>{ty.i32()},
            1_i);
        b.Override("o", comp);
    });

    b.Append(ep->Block(), [&] {  //
        auto* comp = b.CallExplicit<spirv::ir::BuiltinCall>(
            ty.i32(), spirv::BuiltinFn::kNot, Vector<core::ir::TemplateParameter, 1>{ty.i32()},
            1_i);
        b.Let("l", comp);
        b.Return(ep);
    });

    auto src = R"(
$B1: {  # root
  %1:i32 = spirv.not<i32> 1i
  %o:i32 = override %1
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %4:i32 = spirv.not<i32> 1i
    %l:i32 = let %4
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
$B1: {  # root
  %o:i32 = override -2i
}

%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B2: {
    %l:i32 = let -2i
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Scalar_Signed_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", 1_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kNot,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:i32 = let 1i
    %3:i32 = spirv.not<i32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:i32 = let 1i
    %3:i32 = complement %l
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Scalar_Signed_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", 1_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kNot,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:i32 = let 1i
    %3:u32 = spirv.not<u32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:i32 = let 1i
    %3:i32 = complement %l
    %4:u32 = bitcast<u32> %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Scalar_Unsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", 8_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kNot,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:u32 = let 8u
    %3:i32 = spirv.not<i32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:u32 = let 8u
    %3:u32 = complement %l
    %4:i32 = bitcast<i32> %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Vector_Signed_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", b.Splat<vec2<i32>>(1_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kNot,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<i32> = let vec2<i32>(1i)
    %3:vec2<i32> = spirv.not<i32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<i32> = let vec2<i32>(1i)
    %3:vec2<i32> = complement %l
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Vector_Signed_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", b.Splat<vec2<i32>>(1_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kNot,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<i32> = let vec2<i32>(1i)
    %3:vec2<u32> = spirv.not<u32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<i32> = let vec2<i32>(1i)
    %3:vec2<i32> = complement %l
    %4:vec2<u32> = bitcast<vec2<u32>> %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Vector_Unsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", b.Splat<vec2<u32>>(8_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kNot,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<u32> = let vec2<u32>(8u)
    %3:vec2<i32> = spirv.not<i32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<u32> = let vec2<u32>(8u)
    %3:vec2<u32> = complement %l
    %4:vec2<i32> = bitcast<vec2<i32>> %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Not_Vector_Unsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", b.Splat<vec2<u32>>(8_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kNot,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<u32> = let vec2<u32>(8u)
    %3:vec2<u32> = spirv.not<u32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<u32> = let vec2<u32>(8u)
    %3:vec2<u32> = complement %l
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Scalar_Signed_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", 1_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kSNegate,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:i32 = let 1i
    %3:i32 = spirv.s_negate<i32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:i32 = let 1i
    %3:i32 = negation %l
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Scalar_Signed_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", 1_i);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kSNegate,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:i32 = let 1i
    %3:u32 = spirv.s_negate<u32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:i32 = let 1i
    %3:i32 = negation %l
    %4:u32 = bitcast<u32> %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Scalar_Unsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", 8_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kSNegate,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:u32 = let 8u
    %3:i32 = spirv.s_negate<i32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:u32 = let 8u
    %3:i32 = bitcast<i32> %l
    %4:i32 = negation %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Scalar_Unsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", 8_u);
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kSNegate,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:u32 = let 8u
    %3:u32 = spirv.s_negate<u32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:u32 = let 8u
    %3:i32 = bitcast<i32> %l
    %4:i32 = negation %3
    %5:u32 = bitcast<u32> %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Vector_Signed_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", b.Splat<vec2<i32>>(1_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kSNegate,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<i32> = let vec2<i32>(1i)
    %3:vec2<i32> = spirv.s_negate<i32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<i32> = let vec2<i32>(1i)
    %3:vec2<i32> = negation %l
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Vector_Signed_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", b.Splat<vec2<i32>>(1_i));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kSNegate,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<i32> = let vec2<i32>(1i)
    %3:vec2<u32> = spirv.s_negate<u32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<i32> = let vec2<i32>(1i)
    %3:vec2<i32> = negation %l
    %4:vec2<u32> = bitcast<vec2<u32>> %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Vector_Unsigned_Signed) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", b.Splat<vec2<u32>>(8_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2i(), spirv::BuiltinFn::kSNegate,
                                               Vector<core::ir::TemplateParameter, 1>{ty.i32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<u32> = let vec2<u32>(8u)
    %3:vec2<i32> = spirv.s_negate<i32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<u32> = let vec2<u32>(8u)
    %3:vec2<i32> = bitcast<vec2<i32>> %l
    %4:vec2<i32> = negation %3
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, SNegate_Vector_Unsigned_Unsigned) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* l = b.Let("l", b.Splat<vec2<u32>>(8_u));
        b.CallExplicit<spirv::ir::BuiltinCall>(ty.vec2u(), spirv::BuiltinFn::kSNegate,
                                               Vector<core::ir::TemplateParameter, 1>{ty.u32()}, l);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<u32> = let vec2<u32>(8u)
    %3:vec2<u32> = spirv.s_negate<u32> %l
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %l:vec2<u32> = let vec2<u32>(8u)
    %3:vec2<i32> = bitcast<vec2<i32>> %l
    %4:vec2<i32> = negation %3
    %5:vec2<u32> = bitcast<vec2<u32>> %4
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FMod_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", 1_f);
        auto* y = b.Let("y", 2_f);
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kFMod, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:f32 = let 1.0f
    %y:f32 = let 2.0f
    %4:f32 = spirv.f_mod %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:f32 = let 1.0f
    %y:f32 = let 2.0f
    %4:f32 = div %x, %y
    %5:f32 = floor %4
    %6:f32 = mul %y, %5
    %7:f32 = sub %x, %6
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, FMod_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        auto* x = b.Let("x", b.Splat<vec2<f32>>(1_f));
        auto* y = b.Let("y", b.Splat<vec2<f32>>(2_f));
        b.Call<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kFMod, x, y);
        b.Return(ep);
    });

    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<f32> = let vec2<f32>(1.0f)
    %y:vec2<f32> = let vec2<f32>(2.0f)
    %4:vec2<f32> = spirv.f_mod %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec2<f32> = let vec2<f32>(1.0f)
    %y:vec2<f32> = let vec2<f32>(2.0f)
    %4:vec2<f32> = div %x, %y
    %5:vec2<f32> = floor %4
    %6:vec2<f32> = mul %y, %5
    %7:vec2<f32> = sub %x, %6
    ret
  }
}
)";

    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, Select_Scalar) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.f32(), spirv::BuiltinFn::kSelect, true, 1_f, 2_f);
        b.Return(ep);
    });
    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = spirv.select true, 1.0f, 2.0f
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:f32 = select 2.0f, 1.0f, true
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}
TEST_F(SpirvReader_BuiltinsTest, Select_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec2f(), spirv::BuiltinFn::kSelect,
                                       b.Splat<vec2<bool>>(false), b.Splat<vec2<f32>>(1_f),
                                       b.Splat<vec2<f32>>(2_f));
        b.Return(ep);
    });
    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = spirv.select vec2<bool>(false), vec2<f32>(1.0f), vec2<f32>(2.0f)
    ret
  }
}
)";
    EXPECT_EQ(src, str());
    Run(Builtins);

    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec2<f32> = select vec2<f32>(2.0f), vec2<f32>(1.0f), vec2<bool>(false)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, OuterProduct_Vector) {
    auto* ep = b.ComputeFunction("foo");

    b.Append(ep->Block(), [&] {  //
        // Call the OuterProduct builtin function
        auto* x = b.Let("x", b.Splat<vec4<f32>>(1_f));
        auto* y = b.Let("y", b.Splat<vec2<f32>>(2_f));
        b.Call<spirv::ir::BuiltinCall>(ty.mat2x4<f32>(), spirv::BuiltinFn::kOuterProduct, x, y);
        b.Return(ep);
    });

    // Expected SPIR-V source code
    auto src = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec4<f32> = let vec4<f32>(1.0f)
    %y:vec2<f32> = let vec2<f32>(2.0f)
    %4:mat2x4<f32> = spirv.outer_product %x, %y
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    // Run the test
    Run(Builtins);

    // Updated expected expanded SPIR-V code after lowering
    auto expect = R"(
%foo = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %x:vec4<f32> = let vec4<f32>(1.0f)
    %y:vec2<f32> = let vec2<f32>(2.0f)
    %4:f32 = access %y, 0u
    %5:f32 = access %x, 0u
    %6:f32 = mul %5, %4
    %7:f32 = access %x, 1u
    %8:f32 = mul %7, %4
    %9:f32 = access %x, 2u
    %10:f32 = mul %9, %4
    %11:f32 = access %x, 3u
    %12:f32 = mul %11, %4
    %13:vec4<f32> = construct %6, %8, %10, %12
    %14:f32 = access %y, 1u
    %15:f32 = access %x, 0u
    %16:f32 = mul %15, %14
    %17:f32 = access %x, 1u
    %18:f32 = mul %17, %14
    %19:f32 = access %x, 2u
    %20:f32 = mul %19, %14
    %21:f32 = access %x, 3u
    %22:f32 = mul %21, %14
    %23:vec4<f32> = construct %16, %18, %20, %22
    %24:mat2x4<f32> = construct %13, %23
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformBroadcast_Constant_BoolScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), spirv::BuiltinFn::kGroupNonUniformBroadcast, 3_u,
                                       true, 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_broadcast 3u, true, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = convert true
    %3:u32 = subgroupBroadcast %2, 1u
    %4:bool = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformBroadcast_Constant_BoolVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3(ty.bool_()),
                                       spirv::BuiltinFn::kGroupNonUniformBroadcast, 3_u,
                                       b.Composite(ty.vec3(ty.bool_()), true, false, true), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_broadcast 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = convert vec3<bool>(true, false, true)
    %3:vec3<u32> = subgroupBroadcast %2, 1u
    %4:vec3<bool> = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformBroadcast_Constant_NumericScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kGroupNonUniformBroadcast, 3_u,
                                       2_u, 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_broadcast 3u, 2u, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupBroadcast 2u, 1u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformBroadcast_Constant_NumericVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3u(), spirv::BuiltinFn::kGroupNonUniformBroadcast, 3_u,
                                       b.Composite(ty.vec3u(), 2_u, 3_u, 2_u), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_broadcast 3u, vec3<u32>(2u, 3u, 2u), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = subgroupBroadcast vec3<u32>(2u, 3u, 2u), 1u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformBroadcastFirst_Constant_BoolScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), spirv::BuiltinFn::kGroupNonUniformBroadcastFirst,
                                       3_u, true);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_broadcast_first 3u, true
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = convert true
    %3:u32 = subgroupBroadcastFirst %2
    %4:bool = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformBroadcastFirst_Constant_BoolVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3(ty.bool_()),
                                       spirv::BuiltinFn::kGroupNonUniformBroadcastFirst, 3_u,
                                       b.Composite(ty.vec3(ty.bool_()), true, false, true));
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_broadcast_first 3u, vec3<bool>(true, false, true)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = convert vec3<bool>(true, false, true)
    %3:vec3<u32> = subgroupBroadcastFirst %2
    %4:vec3<bool> = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformBroadcastFirst_Constant_NumericScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kGroupNonUniformBroadcastFirst,
                                       3_u, 2_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_broadcast_first 3u, 2u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupBroadcastFirst 2u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformBroadcastFirst_Constant_NumericVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3u(), spirv::BuiltinFn::kGroupNonUniformBroadcastFirst,
                                       3_u, b.Composite(ty.vec3u(), 2_u, 3_u, 2_u));
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_broadcast_first 3u, vec3<u32>(2u, 3u, 2u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = subgroupBroadcastFirst vec3<u32>(2u, 3u, 2u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformQuadBroadcast_Constant_BoolScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), spirv::BuiltinFn::kGroupNonUniformQuadBroadcast,
                                       3_u, true, 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_quad_broadcast 3u, true, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = convert true
    %3:u32 = quadBroadcast %2, 1u
    %4:bool = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformQuadBroadcast_Constant_BoolVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3(ty.bool_()),
                                       spirv::BuiltinFn::kGroupNonUniformQuadBroadcast, 3_u,
                                       b.Composite(ty.vec3(ty.bool_()), true, false, true), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_quad_broadcast 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = convert vec3<bool>(true, false, true)
    %3:vec3<u32> = quadBroadcast %2, 1u
    %4:vec3<bool> = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformQuadBroadcast_Constant_NumericScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kGroupNonUniformQuadBroadcast,
                                       3_u, 2_u, 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_quad_broadcast 3u, 2u, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = quadBroadcast 2u, 1u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformQuadBroadcast_Constant_NumericVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3u(), spirv::BuiltinFn::kGroupNonUniformQuadBroadcast,
                                       3_u, b.Composite(ty.vec3u(), 2_u, 3_u, 2_u), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_quad_broadcast 3u, vec3<u32>(2u, 3u, 2u), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = quadBroadcast vec3<u32>(2u, 3u, 2u), 1u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformQuadSwap_Constant_BoolScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), spirv::BuiltinFn::kGroupNonUniformQuadSwap, 3_u,
                                       true, 0_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_quad_swap 3u, true, 0u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = convert true
    %3:u32 = quadSwapX %2
    %4:bool = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformQuadSwap_Constant_BoolVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3(ty.bool_()),
                                       spirv::BuiltinFn::kGroupNonUniformQuadSwap, 3_u,
                                       b.Composite(ty.vec3(ty.bool_()), true, false, true), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_quad_swap 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = convert vec3<bool>(true, false, true)
    %3:vec3<u32> = quadSwapY %2
    %4:vec3<bool> = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformQuadSwap_Constant_NumericScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kGroupNonUniformQuadSwap, 3_u,
                                       2_u, 2_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_quad_swap 3u, 2u, 2u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = quadSwapDiagonal 2u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformQuadSwap_Constant_NumericVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3u(), spirv::BuiltinFn::kGroupNonUniformQuadSwap, 3_u,
                                       b.Composite(ty.vec3u(), 2_u, 3_u, 2_u), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_quad_swap 3u, vec3<u32>(2u, 3u, 2u), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = quadSwapY vec3<u32>(2u, 3u, 2u)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffle_Constant_BoolScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), spirv::BuiltinFn::kGroupNonUniformShuffle, 3_u,
                                       true, 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_shuffle 3u, true, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = convert true
    %3:u32 = subgroupShuffle %2, 1u
    %4:bool = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffle_Constant_BoolVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3(ty.bool_()),
                                       spirv::BuiltinFn::kGroupNonUniformShuffle, 3_u,
                                       b.Composite(ty.vec3(ty.bool_()), true, false, true), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_shuffle 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = convert vec3<bool>(true, false, true)
    %3:vec3<u32> = subgroupShuffle %2, 1u
    %4:vec3<bool> = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffle_Constant_NumericScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kGroupNonUniformShuffle, 3_u,
                                       2_u, 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_shuffle 3u, 2u, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupShuffle 2u, 1u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffle_Constant_NumericVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3u(), spirv::BuiltinFn::kGroupNonUniformShuffle, 3_u,
                                       b.Composite(ty.vec3u(), 2_u, 3_u, 2_u), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_shuffle 3u, vec3<u32>(2u, 3u, 2u), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = subgroupShuffle vec3<u32>(2u, 3u, 2u), 1u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffleXor_Constant_BoolScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), spirv::BuiltinFn::kGroupNonUniformShuffleXor,
                                       3_u, true, 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_shuffle_xor 3u, true, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = convert true
    %3:u32 = subgroupShuffleXor %2, 1u
    %4:bool = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffleXor_Constant_BoolVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3(ty.bool_()),
                                       spirv::BuiltinFn::kGroupNonUniformShuffleXor, 3_u,
                                       b.Composite(ty.vec3(ty.bool_()), true, false, true), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_shuffle_xor 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = convert vec3<bool>(true, false, true)
    %3:vec3<u32> = subgroupShuffleXor %2, 1u
    %4:vec3<bool> = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffleXor_Constant_NumericScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kGroupNonUniformShuffleXor, 3_u,
                                       2_u, 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_shuffle_xor 3u, 2u, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupShuffleXor 2u, 1u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffleXor_Constant_NumericVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3u(), spirv::BuiltinFn::kGroupNonUniformShuffleXor,
                                       3_u, b.Composite(ty.vec3u(), 2_u, 3_u, 2_u), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_shuffle_xor 3u, vec3<u32>(2u, 3u, 2u), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = subgroupShuffleXor vec3<u32>(2u, 3u, 2u), 1u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffleDown_Constant_BoolScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), spirv::BuiltinFn::kGroupNonUniformShuffleDown,
                                       3_u, true, 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_shuffle_down 3u, true, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = convert true
    %3:u32 = subgroupShuffleDown %2, 1u
    %4:bool = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffleDown_Constant_BoolVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3(ty.bool_()),
                                       spirv::BuiltinFn::kGroupNonUniformShuffleDown, 3_u,
                                       b.Composite(ty.vec3(ty.bool_()), true, false, true), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_shuffle_down 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = convert vec3<bool>(true, false, true)
    %3:vec3<u32> = subgroupShuffleDown %2, 1u
    %4:vec3<bool> = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffleDown_Constant_NumericScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kGroupNonUniformShuffleDown, 3_u,
                                       2_u, 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_shuffle_down 3u, 2u, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupShuffleDown 2u, 1u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffleDown_Constant_NumericVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3u(), spirv::BuiltinFn::kGroupNonUniformShuffleDown,
                                       3_u, b.Composite(ty.vec3u(), 2_u, 3_u, 2_u), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_shuffle_down 3u, vec3<u32>(2u, 3u, 2u), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = subgroupShuffleDown vec3<u32>(2u, 3u, 2u), 1u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffleUp_Constant_BoolScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.bool_(), spirv::BuiltinFn::kGroupNonUniformShuffleUp, 3_u,
                                       true, 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:bool = spirv.group_non_uniform_shuffle_up 3u, true, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = convert true
    %3:u32 = subgroupShuffleUp %2, 1u
    %4:bool = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffleUp_Constant_BoolVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3(ty.bool_()),
                                       spirv::BuiltinFn::kGroupNonUniformShuffleUp, 3_u,
                                       b.Composite(ty.vec3(ty.bool_()), true, false, true), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<bool> = spirv.group_non_uniform_shuffle_up 3u, vec3<bool>(true, false, true), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = convert vec3<bool>(true, false, true)
    %3:vec3<u32> = subgroupShuffleUp %2, 1u
    %4:vec3<bool> = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffleUp_Constant_NumericScalar) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kGroupNonUniformShuffleUp, 3_u,
                                       2_u, 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_shuffle_up 3u, 2u, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = subgroupShuffleUp 2u, 1u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformShuffleUp_Constant_NumericVector) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3u(), spirv::BuiltinFn::kGroupNonUniformShuffleUp, 3_u,
                                       b.Composite(ty.vec3u(), 2_u, 3_u, 2_u), 1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_shuffle_up 3u, vec3<u32>(2u, 3u, 2u), 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = subgroupShuffleUp vec3<u32>(2u, 3u, 2u), 1u
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformSMin_Scalar_i32) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kGroupNonUniformSMin, 3_u, 0_u,
                                       1_i);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.group_non_uniform_s_min 3u, 0u, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = subgroupMin 1i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformSMin_Vector_i32) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3i(), spirv::BuiltinFn::kGroupNonUniformSMin, 3_u, 0_u,
                                       b.Composite(ty.vec3i(), 1_i, 3_i, 1_i));
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = spirv.group_non_uniform_s_min 3u, 0u, vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = subgroupMin vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformSMin_Scalar_u32) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kGroupNonUniformSMin, 3_u, 0_u,
                                       1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_s_min 3u, 0u, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = convert 1u
    %3:i32 = subgroupMin %2
    %4:u32 = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformSMin_Vector_u32) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3u(), spirv::BuiltinFn::kGroupNonUniformSMin, 3_u, 0_u,
                                       b.Composite(ty.vec3u(), 1_u, 3_u, 1_u));
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_s_min 3u, 0u, vec3<u32>(1u, 3u, 1u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = convert vec3<u32>(1u, 3u, 1u)
    %3:vec3<i32> = subgroupMin %2
    %4:vec3<u32> = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformSMax_Scalar_i32) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.i32(), spirv::BuiltinFn::kGroupNonUniformSMax, 3_u, 0_u,
                                       1_i);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = spirv.group_non_uniform_s_max 3u, 0u, 1i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = subgroupMax 1i
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformSMax_Vector_i32) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3i(), spirv::BuiltinFn::kGroupNonUniformSMax, 3_u, 0_u,
                                       b.Composite(ty.vec3i(), 1_i, 3_i, 1_i));
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = spirv.group_non_uniform_s_max 3u, 0u, vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = subgroupMax vec3<i32>(1i, 3i, 1i)
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformSMax_Scalar_u32) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.u32(), spirv::BuiltinFn::kGroupNonUniformSMax, 3_u, 0_u,
                                       1_u);
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:u32 = spirv.group_non_uniform_s_max 3u, 0u, 1u
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:i32 = convert 1u
    %3:i32 = subgroupMax %2
    %4:u32 = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

TEST_F(SpirvReader_BuiltinsTest, NonUniformSMax_Vector_u32) {
    auto* ep = b.ComputeFunction("main");

    b.Append(ep->Block(), [&] {  //
        b.Call<spirv::ir::BuiltinCall>(ty.vec3u(), spirv::BuiltinFn::kGroupNonUniformSMax, 3_u, 0_u,
                                       b.Composite(ty.vec3u(), 1_u, 3_u, 1_u));
        b.Return(ep);
    });

    auto src = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<u32> = spirv.group_non_uniform_s_max 3u, 0u, vec3<u32>(1u, 3u, 1u)
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    Run(Builtins);

    auto expect = R"(
%main = @compute @workgroup_size(1u, 1u, 1u) func():void {
  $B1: {
    %2:vec3<i32> = convert vec3<u32>(1u, 3u, 1u)
    %3:vec3<i32> = subgroupMax %2
    %4:vec3<u32> = convert %3
    ret
  }
}
)";
    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::spirv::reader::lower
