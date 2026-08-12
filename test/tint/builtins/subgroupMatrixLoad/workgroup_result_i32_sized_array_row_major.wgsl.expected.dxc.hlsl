SKIP: FAILED

../../src/tint/lang/core/ir/validator.cc:111 internal compiler error: 
=========================================================
== IR validation failed before core.VectorizeScalarMatrixConstructors:
=========================================================
:107:45 error: hlsl.Load: no matching call to 'hlsl.Load<subgroup_matrix_result<i32, 8, 8>>(ptr<workgroup, array<vec2<i32>, 1024>, read_write>, u32, u32, hlsl.matrix_layout)'

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

    %74:subgroup_matrix_result<i32, 8, 8> = hlsl.Load<subgroup_matrix_result<i32, 8, 8>> %in1, 0u, 16u, 0u
                                            ^^^^^^^^^

:16:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
main_inputs = struct @align(4) {
  tint_local_index:u32 @offset(0), @builtin(local_invocation_index)
}

$B1: {  # root
  %in0:ptr<workgroup, array<u32, 1024>, read_write> = var undef
  %in1:ptr<workgroup, array<vec2<i32>, 1024>, read_write> = var undef
  %in2:ptr<workgroup, array<u32, 4096>, read_write> = var undef
  %in3:ptr<workgroup, array<vec4<u32>, 1024>, read_write> = var undef
  %in5:ptr<workgroup, array<u16, 2048>, read_write> = var undef
  %in6:ptr<workgroup, array<u16, 4096>, read_write> = var undef
  %out:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
}

%main_inner = func(%tint_local_index:u32):void {
  $B2: {
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        next_iteration %tint_local_index  # -> $B4
      }
      $B4 (%idx:u32): {  # body
        %11:bool = gte %idx, 1024u
        if %11 [t: $B6] {  # if_1
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %12:u32 = mul %idx, 4u
        %13:u32 = div %12, 4u
        %14:ptr<workgroup, u32, read_write> = access %in0, %13
        store %14, 0u
        %15:ptr<workgroup, vec2<i32>, read_write> = access %in1, %idx
        store %15, vec2<i32>(0i)
        %16:u32 = mul %idx, 16u
        %17:u32 = div %16, 4u
        %18:f32 = access vec3<f32>(0.0f), 0u
        %19:u32 = hlsl.asuint %18
        %20:ptr<workgroup, u32, read_write> = access %in2, %17
        store %20, %19
        %21:u32 = add %17, 1u
        %22:f32 = access vec3<f32>(0.0f), 1u
        %23:u32 = hlsl.asuint %22
        %24:ptr<workgroup, u32, read_write> = access %in2, %21
        store %24, %23
        %25:u32 = add %21, 1u
        %26:f32 = access vec3<f32>(0.0f), 2u
        %27:u32 = hlsl.asuint %26
        %28:ptr<workgroup, u32, read_write> = access %in2, %25
        store %28, %27
        %29:u32 = mul %idx, 16u
        %30:u32 = div %29, 16u
        %31:ptr<workgroup, vec4<u32>, read_write> = access %in3, %30
        store %31, vec4<u32>(0u)
        %32:u32 = mul %idx, 4u
        %33:u32 = div %32, 2u
        %34:f16 = access vec2<f16>(0.0h), 0u
        %35:u16 = hlsl.asuint16 %34
        %36:ptr<workgroup, u16, read_write> = access %in5, %33
        store %36, %35
        %37:u32 = add %33, 1u
        %38:f16 = access vec2<f16>(0.0h), 1u
        %39:u16 = hlsl.asuint16 %38
        %40:ptr<workgroup, u16, read_write> = access %in5, %37
        store %40, %39
        %41:u32 = mul %idx, 8u
        %42:u32 = div %41, 2u
        %43:f16 = access vec3<f16>(0.0h), 0u
        %44:u16 = hlsl.asuint16 %43
        %45:ptr<workgroup, u16, read_write> = access %in6, %42
        store %45, %44
        %46:u32 = add %42, 1u
        %47:f16 = access vec3<f16>(0.0h), 1u
        %48:u16 = hlsl.asuint16 %47
        %49:ptr<workgroup, u16, read_write> = access %in6, %46
        store %49, %48
        %50:u32 = add %46, 1u
        %51:f16 = access vec3<f16>(0.0h), 2u
        %52:u16 = hlsl.asuint16 %51
        %53:ptr<workgroup, u16, read_write> = access %in6, %50
        store %53, %52
        continue  # -> $B5
      }
      $B5: {  # continuing
        %54:u32 = add %idx, 64u
        next_iteration %54  # -> $B4
      }
    }
    %55:void = workgroupBarrier
    %56:subgroup_matrix_result<i32, 8, 8> = hlsl.Load<subgroup_matrix_result<i32, 8, 8>> %in0, 0u, 16u, 0u
    %m0:subgroup_matrix_result<i32, 8, 8> = let %56
    %58:ptr<function, u32, read_write> = var undef
    %59:void = %out.GetDimensions %58
    %60:u32 = load %58
    %61:u32 = div %60, 4u
    %62:u32 = hlsl.asuint 0i
    %63:u32 = hlsl.asuint 16i
    %64:u32 = mul %63, 7u
    %65:u32 = add %62, %64
    %66:u32 = add %65, 8u
    %67:bool = lte %66, %61
    %68:u32 = hlsl.select %67, %62, 0u
    %69:u32 = hlsl.select %67, %63, 8u
    %70:u32 = mul %68, 4u
    %71:u32 = mul %69, 4u
    %72:u32 = add 0u, %70
    %73:void = %m0.Store %out, %72, %71, 1u
    %74:subgroup_matrix_result<i32, 8, 8> = hlsl.Load<subgroup_matrix_result<i32, 8, 8>> %in1, 0u, 16u, 0u
    %m1:subgroup_matrix_result<i32, 8, 8> = let %74
    %76:ptr<function, u32, read_write> = var undef
    %77:void = %out.GetDimensions %76
    %78:u32 = load %76
    %79:u32 = div %78, 4u
    %80:u32 = hlsl.asuint 0i
    %81:u32 = hlsl.asuint 16i
    %82:u32 = mul %81, 7u
    %83:u32 = add %80, %82
    %84:u32 = add %83, 8u
    %85:bool = lte %84, %79
    %86:u32 = hlsl.select %85, %80, 0u
    %87:u32 = hlsl.select %85, %81, 8u
    %88:u32 = mul %86, 4u
    %89:u32 = mul %87, 4u
    %90:u32 = add 0u, %88
    %91:void = %m1.Store %out, %90, %89, 1u
    %92:u32 = div 0u, 4u
    %93:u32 = mul 0u, 16u
    %94:u32 = div %93, 4u
    %95:u32 = add %92, %94
    %96:u32 = mul 16u, 16u
    %97:u32 = div %96, 4u
    %98:subgroup_matrix_result<i32, 8, 8> = hlsl.Load<subgroup_matrix_result<i32, 8, 8>> %in2, %95, %97, 0u
    %m2:subgroup_matrix_result<i32, 8, 8> = let %98
    %100:ptr<function, u32, read_write> = var undef
    %101:void = %out.GetDimensions %100
    %102:u32 = load %100
    %103:u32 = div %102, 4u
    %104:u32 = hlsl.asuint 0i
    %105:u32 = hlsl.asuint 16i
    %106:u32 = mul %105, 7u
    %107:u32 = add %104, %106
    %108:u32 = add %107, 8u
    %109:bool = lte %108, %103
    %110:u32 = hlsl.select %109, %104, 0u
    %111:u32 = hlsl.select %109, %105, 8u
    %112:u32 = mul %110, 4u
    %113:u32 = mul %111, 4u
    %114:u32 = add 0u, %112
    %115:void = %m2.Store %out, %114, %113, 1u
    %116:subgroup_matrix_result<i32, 8, 8> = hlsl.Load<subgroup_matrix_result<i32, 8, 8>> %in3, 0u, 16u, 0u
    %m3:subgroup_matrix_result<i32, 8, 8> = let %116
    %118:ptr<function, u32, read_write> = var undef
    %119:void = %out.GetDimensions %118
    %120:u32 = load %118
    %121:u32 = div %120, 4u
    %122:u32 = hlsl.asuint 0i
    %123:u32 = hlsl.asuint 16i
    %124:u32 = mul %123, 7u
    %125:u32 = add %122, %124
    %126:u32 = add %125, 8u
    %127:bool = lte %126, %121
    %128:u32 = hlsl.select %127, %122, 0u
    %129:u32 = hlsl.select %127, %123, 8u
    %130:u32 = mul %128, 4u
    %131:u32 = mul %129, 4u
    %132:u32 = add 0u, %130
    %133:void = %m3.Store %out, %132, %131, 1u
    %134:u32 = div 0u, 2u
    %135:u32 = mul 0u, 4u
    %136:u32 = div %135, 2u
    %137:u32 = add %134, %136
    %138:u32 = mul 16u, 4u
    %139:u32 = div %138, 2u
    %140:subgroup_matrix_result<i32, 8, 8> = hlsl.Load<subgroup_matrix_result<i32, 8, 8>> %in5, %137, %139, 0u
    %m5:subgroup_matrix_result<i32, 8, 8> = let %140
    %142:ptr<function, u32, read_write> = var undef
    %143:void = %out.GetDimensions %142
    %144:u32 = load %142
    %145:u32 = div %144, 4u
    %146:u32 = hlsl.asuint 0i
    %147:u32 = hlsl.asuint 16i
    %148:u32 = mul %147, 7u
    %149:u32 = add %146, %148
    %150:u32 = add %149, 8u
    %151:bool = lte %150, %145
    %152:u32 = hlsl.select %151, %146, 0u
    %153:u32 = hlsl.select %151, %147, 8u
    %154:u32 = mul %152, 4u
    %155:u32 = mul %153, 4u
    %156:u32 = add 0u, %154
    %157:void = %m5.Store %out, %156, %155, 1u
    %158:u32 = div 0u, 2u
    %159:u32 = mul 0u, 8u
    %160:u32 = div %159, 2u
    %161:u32 = add %158, %160
    %162:u32 = mul 16u, 8u
    %163:u32 = div %162, 2u
    %164:subgroup_matrix_result<i32, 8, 8> = hlsl.Load<subgroup_matrix_result<i32, 8, 8>> %in6, %161, %163, 0u
    %m6:subgroup_matrix_result<i32, 8, 8> = let %164
    %166:ptr<function, u32, read_write> = var undef
    %167:void = %out.GetDimensions %166
    %168:u32 = load %166
    %169:u32 = div %168, 4u
    %170:u32 = hlsl.asuint 0i
    %171:u32 = hlsl.asuint 16i
    %172:u32 = mul %171, 7u
    %173:u32 = add %170, %172
    %174:u32 = add %173, 8u
    %175:bool = lte %174, %169
    %176:u32 = hlsl.select %175, %170, 0u
    %177:u32 = hlsl.select %175, %171, 8u
    %178:u32 = mul %176, 4u
    %179:u32 = mul %177, 4u
    %180:u32 = add 0u, %178
    %181:void = %m6.Store %out, %180, %179, 1u
    ret
  }
}
%main = @compute @workgroup_size(64i, 1i, 1i) func(%inputs:main_inputs):void {
  $B7: {
    %184:u32 = access %inputs, 0u
    %185:void = call %main_inner, %184
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
