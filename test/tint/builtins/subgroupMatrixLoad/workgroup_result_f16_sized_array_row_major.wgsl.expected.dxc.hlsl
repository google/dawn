SKIP: FAILED

../../src/tint/lang/core/ir/validator.cc:111 internal compiler error: 
=========================================================
== IR validation failed before core.VectorizeScalarMatrixConstructors:
=========================================================
:189:46 error: hlsl.Load: no matching call to 'hlsl.Load<subgroup_matrix_result<f16, 8, 8>>(ptr<workgroup, array<vec2<f16>, 1024>, read_write>, u32, u32, hlsl.matrix_layout)'

4 candidate functions:
 • 'hlsl.Load<SM  ✓ >(ptr<workgroup, array<AE, AC>, AM>  ✗ , offset: u32  ✓ , stride: u32  ✓ , matrix_layout  ✓ ) -> SM' where:
      ✓  'SM' is 'subgroup_matrix<K, S, C, R>'
      ✓  'S' is 'f32', 'i32', 'u32', 'f16', 'i8' or 'u8'
      ✗  'AE' is 'u16' or 'u32'
      ✓  'AM' is 'read' or 'read_write'
 • 'hlsl.Load<SM  ✓ >(ptr<workgroup, array<vecN<AE>, AC>, AM>  ✗ , offset: u32  ✓ , stride: u32  ✓ , matrix_layout  ✓ ) -> SM' where:
      ✓  'SM' is 'subgroup_matrix<K, S, C, R>'
      ✓  'S' is 'f32', 'i32', 'u32', 'f16', 'i8' or 'u8'
      ✗  'AE' is 'u16' or 'u32'
      ✓  'AM' is 'read' or 'read_write'
 • 'hlsl.Load<SM  ✓ >(byte_address_buffer<AM>  ✗ , offset: u32  ✓ , stride: u32  ✓ , matrix_layout  ✓ ) -> SM' where:
      ✓  'SM' is 'subgroup_matrix<K, S, C, R>'
      ✓  'S' is 'f32', 'i32', 'u32', 'f16', 'i8' or 'u8'
      ✗  'AM' is 'read' or 'read_write'
 • 'hlsl.Load<SM  ✓ >(ptr<workgroup, array<S, AC>, AM>  ✗ , offset: u32  ✓ , stride: u32  ✓ , matrix_layout  ✓ ) -> SM' where:
      ✓  'SM' is 'subgroup_matrix<K, S, C, R>'
      ✓  'S' is 'f32', 'i32', 'u32', 'f16', 'i8' or 'u8'
      ✗  'AM' is 'read' or 'read_write'

    %159:subgroup_matrix_result<f16, 8, 8> = hlsl.Load<subgroup_matrix_result<f16, 8, 8>> %in5, 0u, 16u, 0u
                                             ^^^^^^^^^

:17:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
main_inputs = struct @align(4) {
  tint_local_index:u32 @offset(0), @builtin(local_invocation_index)
}

$B1: {  # root
  %in0:ptr<workgroup, array<u32, 1024>, read_write> = var undef
  %in1:ptr<workgroup, array<vec2<u32>, 1024>, read_write> = var undef
  %in2:ptr<workgroup, array<u32, 4096>, read_write> = var undef
  %in3:ptr<workgroup, array<vec4<u32>, 1024>, read_write> = var undef
  %in4:ptr<workgroup, array<f16, 1024>, read_write> = var undef
  %in5:ptr<workgroup, array<vec2<f16>, 1024>, read_write> = var undef
  %in6:ptr<workgroup, array<vec3<f16>, 1024>, read_write> = var undef
  %out:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%main_inner = func(%tint_local_index:u32):void {
  $B2: {
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        next_iteration %tint_local_index  # -> $B4
      }
      $B4 (%idx:u32): {  # body
        %12:bool = gte %idx, 1024u
        if %12 [t: $B6] {  # if_1
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %13:u32 = mul %idx, 4u
        %14:u32 = div %13, 4u
        %15:ptr<workgroup, u32, read_write> = access %in0, %14
        store %15, 0u
        %16:u32 = mul %idx, 8u
        %17:u32 = div %16, 8u
        %18:vec2<i32> = construct vec2<i32>(0i)
        %19:vec2<u32> = hlsl.asuint %18
        %20:ptr<workgroup, vec2<u32>, read_write> = access %in1, %17
        store %20, %19
        %21:u32 = mul %idx, 16u
        %22:u32 = div %21, 4u
        %23:f32 = access vec3<f32>(0.0f), 0u
        %24:u32 = hlsl.asuint %23
        %25:ptr<workgroup, u32, read_write> = access %in2, %22
        store %25, %24
        %26:u32 = add %22, 1u
        %27:f32 = access vec3<f32>(0.0f), 1u
        %28:u32 = hlsl.asuint %27
        %29:ptr<workgroup, u32, read_write> = access %in2, %26
        store %29, %28
        %30:u32 = add %26, 1u
        %31:f32 = access vec3<f32>(0.0f), 2u
        %32:u32 = hlsl.asuint %31
        %33:ptr<workgroup, u32, read_write> = access %in2, %30
        store %33, %32
        %34:u32 = mul %idx, 16u
        %35:u32 = div %34, 16u
        %36:ptr<workgroup, vec4<u32>, read_write> = access %in3, %35
        store %36, vec4<u32>(0u)
        %37:ptr<workgroup, f16, read_write> = access %in4, %idx
        store %37, 0.0h
        %38:ptr<workgroup, vec2<f16>, read_write> = access %in5, %idx
        store %38, vec2<f16>(0.0h)
        %39:ptr<workgroup, vec3<f16>, read_write> = access %in6, %idx
        store %39, vec3<f16>(0.0h)
        continue  # -> $B5
      }
      $B5: {  # continuing
        %40:u32 = add %idx, 64u
        next_iteration %40  # -> $B4
      }
    }
    %41:void = workgroupBarrier
    %42:subgroup_matrix_result<f16, 8, 8> = hlsl.Load<subgroup_matrix_result<f16, 8, 8>> %in0, 0u, 16u, 0u
    %m0:subgroup_matrix_result<f16, 8, 8> = let %42
    %44:ptr<function, u32, read_write> = var undef
    %45:void = %out.GetDimensions %44
    %46:u32 = load %44
    %47:u32 = div %46, 4u
    %48:u32 = mul %47, 4u
    %49:u32 = call %tint_div_u32, %48, 2u
    %51:u32 = hlsl.asuint 0i
    %52:u32 = hlsl.asuint 16i
    %53:u32 = mul %52, 7u
    %54:u32 = add %51, %53
    %55:u32 = mul %54, 4u
    %56:u32 = call %tint_div_u32, %55, 2u
    %57:u32 = add %56, 8u
    %58:bool = lte %57, %49
    %59:u32 = hlsl.select %58, %51, 0u
    %60:u32 = hlsl.select %58, %52, 4u
    %61:u32 = mul %59, 2u
    %62:u32 = mul %60, 2u
    %63:u32 = add 0u, %61
    %64:void = %m0.Store %out, %63, %62, 1u
    %65:subgroup_matrix_result<f16, 8, 8> = hlsl.Load<subgroup_matrix_result<f16, 8, 8>> %in1, 0u, 16u, 0u
    %m1:subgroup_matrix_result<f16, 8, 8> = let %65
    %67:ptr<function, u32, read_write> = var undef
    %68:void = %out.GetDimensions %67
    %69:u32 = load %67
    %70:u32 = div %69, 4u
    %71:u32 = mul %70, 4u
    %72:u32 = call %tint_div_u32, %71, 2u
    %73:u32 = hlsl.asuint 0i
    %74:u32 = hlsl.asuint 16i
    %75:u32 = mul %74, 7u
    %76:u32 = add %73, %75
    %77:u32 = mul %76, 4u
    %78:u32 = call %tint_div_u32, %77, 2u
    %79:u32 = add %78, 8u
    %80:bool = lte %79, %72
    %81:u32 = hlsl.select %80, %73, 0u
    %82:u32 = hlsl.select %80, %74, 4u
    %83:u32 = mul %81, 2u
    %84:u32 = mul %82, 2u
    %85:u32 = add 0u, %83
    %86:void = %m1.Store %out, %85, %84, 1u
    %87:u32 = div 0u, 4u
    %88:u32 = mul 0u, 16u
    %89:u32 = div %88, 4u
    %90:u32 = add %87, %89
    %91:u32 = mul 16u, 16u
    %92:u32 = div %91, 4u
    %93:subgroup_matrix_result<f16, 8, 8> = hlsl.Load<subgroup_matrix_result<f16, 8, 8>> %in2, %90, %92, 0u
    %m2:subgroup_matrix_result<f16, 8, 8> = let %93
    %95:ptr<function, u32, read_write> = var undef
    %96:void = %out.GetDimensions %95
    %97:u32 = load %95
    %98:u32 = div %97, 4u
    %99:u32 = mul %98, 4u
    %100:u32 = call %tint_div_u32, %99, 2u
    %101:u32 = hlsl.asuint 0i
    %102:u32 = hlsl.asuint 16i
    %103:u32 = mul %102, 7u
    %104:u32 = add %101, %103
    %105:u32 = mul %104, 4u
    %106:u32 = call %tint_div_u32, %105, 2u
    %107:u32 = add %106, 8u
    %108:bool = lte %107, %100
    %109:u32 = hlsl.select %108, %101, 0u
    %110:u32 = hlsl.select %108, %102, 4u
    %111:u32 = mul %109, 2u
    %112:u32 = mul %110, 2u
    %113:u32 = add 0u, %111
    %114:void = %m2.Store %out, %113, %112, 1u
    %115:subgroup_matrix_result<f16, 8, 8> = hlsl.Load<subgroup_matrix_result<f16, 8, 8>> %in3, 0u, 16u, 0u
    %m3:subgroup_matrix_result<f16, 8, 8> = let %115
    %117:ptr<function, u32, read_write> = var undef
    %118:void = %out.GetDimensions %117
    %119:u32 = load %117
    %120:u32 = div %119, 4u
    %121:u32 = mul %120, 4u
    %122:u32 = call %tint_div_u32, %121, 2u
    %123:u32 = hlsl.asuint 0i
    %124:u32 = hlsl.asuint 16i
    %125:u32 = mul %124, 7u
    %126:u32 = add %123, %125
    %127:u32 = mul %126, 4u
    %128:u32 = call %tint_div_u32, %127, 2u
    %129:u32 = add %128, 8u
    %130:bool = lte %129, %122
    %131:u32 = hlsl.select %130, %123, 0u
    %132:u32 = hlsl.select %130, %124, 4u
    %133:u32 = mul %131, 2u
    %134:u32 = mul %132, 2u
    %135:u32 = add 0u, %133
    %136:void = %m3.Store %out, %135, %134, 1u
    %137:subgroup_matrix_result<f16, 8, 8> = hlsl.Load<subgroup_matrix_result<f16, 8, 8>> %in4, 0u, 16u, 0u
    %m4:subgroup_matrix_result<f16, 8, 8> = let %137
    %139:ptr<function, u32, read_write> = var undef
    %140:void = %out.GetDimensions %139
    %141:u32 = load %139
    %142:u32 = div %141, 4u
    %143:u32 = mul %142, 4u
    %144:u32 = call %tint_div_u32, %143, 2u
    %145:u32 = hlsl.asuint 0i
    %146:u32 = hlsl.asuint 16i
    %147:u32 = mul %146, 7u
    %148:u32 = add %145, %147
    %149:u32 = mul %148, 4u
    %150:u32 = call %tint_div_u32, %149, 2u
    %151:u32 = add %150, 8u
    %152:bool = lte %151, %144
    %153:u32 = hlsl.select %152, %145, 0u
    %154:u32 = hlsl.select %152, %146, 4u
    %155:u32 = mul %153, 2u
    %156:u32 = mul %154, 2u
    %157:u32 = add 0u, %155
    %158:void = %m4.Store %out, %157, %156, 1u
    %159:subgroup_matrix_result<f16, 8, 8> = hlsl.Load<subgroup_matrix_result<f16, 8, 8>> %in5, 0u, 16u, 0u
    %m5:subgroup_matrix_result<f16, 8, 8> = let %159
    %161:ptr<function, u32, read_write> = var undef
    %162:void = %out.GetDimensions %161
    %163:u32 = load %161
    %164:u32 = div %163, 4u
    %165:u32 = mul %164, 4u
    %166:u32 = call %tint_div_u32, %165, 2u
    %167:u32 = hlsl.asuint 0i
    %168:u32 = hlsl.asuint 16i
    %169:u32 = mul %168, 7u
    %170:u32 = add %167, %169
    %171:u32 = mul %170, 4u
    %172:u32 = call %tint_div_u32, %171, 2u
    %173:u32 = add %172, 8u
    %174:bool = lte %173, %166
    %175:u32 = hlsl.select %174, %167, 0u
    %176:u32 = hlsl.select %174, %168, 4u
    %177:u32 = mul %175, 2u
    %178:u32 = mul %176, 2u
    %179:u32 = add 0u, %177
    %180:void = %m5.Store %out, %179, %178, 1u
    %181:subgroup_matrix_result<f16, 8, 8> = hlsl.Load<subgroup_matrix_result<f16, 8, 8>> %in6, 0u, 16u, 0u
    %m6:subgroup_matrix_result<f16, 8, 8> = let %181
    %183:ptr<function, u32, read_write> = var undef
    %184:void = %out.GetDimensions %183
    %185:u32 = load %183
    %186:u32 = div %185, 4u
    %187:u32 = mul %186, 4u
    %188:u32 = call %tint_div_u32, %187, 2u
    %189:u32 = hlsl.asuint 0i
    %190:u32 = hlsl.asuint 16i
    %191:u32 = mul %190, 7u
    %192:u32 = add %189, %191
    %193:u32 = mul %192, 4u
    %194:u32 = call %tint_div_u32, %193, 2u
    %195:u32 = add %194, 8u
    %196:bool = lte %195, %188
    %197:u32 = hlsl.select %196, %189, 0u
    %198:u32 = hlsl.select %196, %190, 4u
    %199:u32 = mul %197, 2u
    %200:u32 = mul %198, 2u
    %201:u32 = add 0u, %199
    %202:void = %m6.Store %out, %201, %200, 1u
    ret
  }
}
%tint_div_u32 = func(%lhs:u32, %rhs:u32):u32 {
  $B7: {
    %205:bool = eq %rhs, 0u
    %206:u32 = hlsl.select %205, 1u, %rhs
    %207:u32 = div %lhs, %206
    ret %207
  }
}
%main = @compute @workgroup_size(64i, 1i, 1i) func(%inputs:main_inputs):void {
  $B8: {
    %210:u32 = access %inputs, 0u
    %211:void = call %main_inner, %210
    ret
  }
}


********************************************************************
*  The tint shader compiler has encountered an unexpected error.   *
*                                                                  *
*  Please help us fix this issue by submitting a bug report at     *
*  crbug.com/tint with the source program that triggered the bug.  *
********************************************************************

tint executable returned error: signal: trace/BPT trap
