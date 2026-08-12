SKIP: FAILED

../../src/tint/lang/core/ir/validator.cc:111 internal compiler error: 
=========================================================
== IR validation failed before core.VectorizeScalarMatrixConstructors:
=========================================================
:118:44 error: hlsl.Load: no matching call to 'hlsl.Load<subgroup_matrix_right<f32, 8, 8>>(ptr<workgroup, array<vec3<f32>, 1024>, read_write>, u32, u32, hlsl.matrix_layout)'

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

    %87:subgroup_matrix_right<f32, 8, 8> = hlsl.Load<subgroup_matrix_right<f32, 8, 8>> %in2, %86, 16u, 1u
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
  %in1:ptr<workgroup, array<vec2<u32>, 1024>, read_write> = var undef
  %in2:ptr<workgroup, array<vec3<f32>, 1024>, read_write> = var undef
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
        %15:u32 = mul %idx, 8u
        %16:u32 = div %15, 8u
        %17:vec2<i32> = construct vec2<i32>(0i)
        %18:vec2<u32> = hlsl.asuint %17
        %19:ptr<workgroup, vec2<u32>, read_write> = access %in1, %16
        store %19, %18
        %20:ptr<workgroup, vec3<f32>, read_write> = access %in2, %idx
        store %20, vec3<f32>(0.0f)
        %21:u32 = mul %idx, 16u
        %22:u32 = div %21, 16u
        %23:ptr<workgroup, vec4<u32>, read_write> = access %in3, %22
        store %23, vec4<u32>(0u)
        %24:u32 = mul %idx, 4u
        %25:u32 = div %24, 2u
        %26:f16 = access vec2<f16>(0.0h), 0u
        %27:u16 = hlsl.asuint16 %26
        %28:ptr<workgroup, u16, read_write> = access %in5, %25
        store %28, %27
        %29:u32 = add %25, 1u
        %30:f16 = access vec2<f16>(0.0h), 1u
        %31:u16 = hlsl.asuint16 %30
        %32:ptr<workgroup, u16, read_write> = access %in5, %29
        store %32, %31
        %33:u32 = mul %idx, 8u
        %34:u32 = div %33, 2u
        %35:f16 = access vec3<f16>(0.0h), 0u
        %36:u16 = hlsl.asuint16 %35
        %37:ptr<workgroup, u16, read_write> = access %in6, %34
        store %37, %36
        %38:u32 = add %34, 1u
        %39:f16 = access vec3<f16>(0.0h), 1u
        %40:u16 = hlsl.asuint16 %39
        %41:ptr<workgroup, u16, read_write> = access %in6, %38
        store %41, %40
        %42:u32 = add %38, 1u
        %43:f16 = access vec3<f16>(0.0h), 2u
        %44:u16 = hlsl.asuint16 %43
        %45:ptr<workgroup, u16, read_write> = access %in6, %42
        store %45, %44
        continue  # -> $B5
      }
      $B5: {  # continuing
        %46:u32 = add %idx, 64u
        next_iteration %46  # -> $B4
      }
    }
    %47:void = workgroupBarrier
    %48:u32 = hlsl.asuint 0i
    %49:subgroup_matrix_right<f32, 8, 8> = hlsl.Load<subgroup_matrix_right<f32, 8, 8>> %in0, %48, 16u, 1u
    %m0:subgroup_matrix_right<f32, 8, 8> = let %49
    %51:ptr<function, u32, read_write> = var undef
    %52:void = %out.GetDimensions %51
    %53:u32 = load %51
    %54:u32 = div %53, 4u
    %55:u32 = hlsl.asuint 0i
    %56:u32 = hlsl.asuint 16i
    %57:u32 = mul %56, 7u
    %58:u32 = add %55, %57
    %59:u32 = add %58, 8u
    %60:bool = lte %59, %54
    %61:u32 = hlsl.select %60, %55, 0u
    %62:u32 = hlsl.select %60, %56, 8u
    %63:u32 = mul %61, 4u
    %64:u32 = mul %62, 4u
    %65:u32 = add 0u, %63
    %66:void = %m0.Store %out, %65, %64, 1u
    %67:u32 = hlsl.asuint 0i
    %68:subgroup_matrix_right<f32, 8, 8> = hlsl.Load<subgroup_matrix_right<f32, 8, 8>> %in1, %67, 16u, 1u
    %m1:subgroup_matrix_right<f32, 8, 8> = let %68
    %70:ptr<function, u32, read_write> = var undef
    %71:void = %out.GetDimensions %70
    %72:u32 = load %70
    %73:u32 = div %72, 4u
    %74:u32 = hlsl.asuint 0i
    %75:u32 = hlsl.asuint 16i
    %76:u32 = mul %75, 7u
    %77:u32 = add %74, %76
    %78:u32 = add %77, 8u
    %79:bool = lte %78, %73
    %80:u32 = hlsl.select %79, %74, 0u
    %81:u32 = hlsl.select %79, %75, 8u
    %82:u32 = mul %80, 4u
    %83:u32 = mul %81, 4u
    %84:u32 = add 0u, %82
    %85:void = %m1.Store %out, %84, %83, 1u
    %86:u32 = hlsl.asuint 0i
    %87:subgroup_matrix_right<f32, 8, 8> = hlsl.Load<subgroup_matrix_right<f32, 8, 8>> %in2, %86, 16u, 1u
    %m2:subgroup_matrix_right<f32, 8, 8> = let %87
    %89:ptr<function, u32, read_write> = var undef
    %90:void = %out.GetDimensions %89
    %91:u32 = load %89
    %92:u32 = div %91, 4u
    %93:u32 = hlsl.asuint 0i
    %94:u32 = hlsl.asuint 16i
    %95:u32 = mul %94, 7u
    %96:u32 = add %93, %95
    %97:u32 = add %96, 8u
    %98:bool = lte %97, %92
    %99:u32 = hlsl.select %98, %93, 0u
    %100:u32 = hlsl.select %98, %94, 8u
    %101:u32 = mul %99, 4u
    %102:u32 = mul %100, 4u
    %103:u32 = add 0u, %101
    %104:void = %m2.Store %out, %103, %102, 1u
    %105:u32 = hlsl.asuint 0i
    %106:subgroup_matrix_right<f32, 8, 8> = hlsl.Load<subgroup_matrix_right<f32, 8, 8>> %in3, %105, 16u, 1u
    %m3:subgroup_matrix_right<f32, 8, 8> = let %106
    %108:ptr<function, u32, read_write> = var undef
    %109:void = %out.GetDimensions %108
    %110:u32 = load %108
    %111:u32 = div %110, 4u
    %112:u32 = hlsl.asuint 0i
    %113:u32 = hlsl.asuint 16i
    %114:u32 = mul %113, 7u
    %115:u32 = add %112, %114
    %116:u32 = add %115, 8u
    %117:bool = lte %116, %111
    %118:u32 = hlsl.select %117, %112, 0u
    %119:u32 = hlsl.select %117, %113, 8u
    %120:u32 = mul %118, 4u
    %121:u32 = mul %119, 4u
    %122:u32 = add 0u, %120
    %123:void = %m3.Store %out, %122, %121, 1u
    %124:u32 = div 0u, 2u
    %125:u32 = hlsl.asuint 0i
    %126:u32 = mul %125, 4u
    %127:u32 = div %126, 2u
    %128:u32 = add %124, %127
    %129:u32 = mul 16u, 4u
    %130:u32 = div %129, 2u
    %131:subgroup_matrix_right<f32, 8, 8> = hlsl.Load<subgroup_matrix_right<f32, 8, 8>> %in5, %128, %130, 1u
    %m5:subgroup_matrix_right<f32, 8, 8> = let %131
    %133:ptr<function, u32, read_write> = var undef
    %134:void = %out.GetDimensions %133
    %135:u32 = load %133
    %136:u32 = div %135, 4u
    %137:u32 = hlsl.asuint 0i
    %138:u32 = hlsl.asuint 16i
    %139:u32 = mul %138, 7u
    %140:u32 = add %137, %139
    %141:u32 = add %140, 8u
    %142:bool = lte %141, %136
    %143:u32 = hlsl.select %142, %137, 0u
    %144:u32 = hlsl.select %142, %138, 8u
    %145:u32 = mul %143, 4u
    %146:u32 = mul %144, 4u
    %147:u32 = add 0u, %145
    %148:void = %m5.Store %out, %147, %146, 1u
    %149:u32 = div 0u, 2u
    %150:u32 = hlsl.asuint 0i
    %151:u32 = mul %150, 8u
    %152:u32 = div %151, 2u
    %153:u32 = add %149, %152
    %154:u32 = mul 16u, 8u
    %155:u32 = div %154, 2u
    %156:subgroup_matrix_right<f32, 8, 8> = hlsl.Load<subgroup_matrix_right<f32, 8, 8>> %in6, %153, %155, 1u
    %m6:subgroup_matrix_right<f32, 8, 8> = let %156
    %158:ptr<function, u32, read_write> = var undef
    %159:void = %out.GetDimensions %158
    %160:u32 = load %158
    %161:u32 = div %160, 4u
    %162:u32 = hlsl.asuint 0i
    %163:u32 = hlsl.asuint 16i
    %164:u32 = mul %163, 7u
    %165:u32 = add %162, %164
    %166:u32 = add %165, 8u
    %167:bool = lte %166, %161
    %168:u32 = hlsl.select %167, %162, 0u
    %169:u32 = hlsl.select %167, %163, 8u
    %170:u32 = mul %168, 4u
    %171:u32 = mul %169, 4u
    %172:u32 = add 0u, %170
    %173:void = %m6.Store %out, %172, %171, 1u
    ret
  }
}
%main = @compute @workgroup_size(64i, 1i, 1i) func(%inputs:main_inputs):void {
  $B7: {
    %176:u32 = access %inputs, 0u
    %177:void = call %main_inner, %176
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
