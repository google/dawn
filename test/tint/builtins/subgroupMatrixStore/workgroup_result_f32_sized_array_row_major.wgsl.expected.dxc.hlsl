SKIP: FAILED

../../src/tint/lang/core/ir/validator.cc:111 internal compiler error: 
=========================================================
== IR validation failed before core.VectorizeScalarMatrixConstructors:
=========================================================
:85:19 error: Store: no matching call to 'Store(subgroup_matrix_result<f32, 8, 8>, ptr<workgroup, array<vec3<f32>, 1024>, read_write>, u32, u32, hlsl.matrix_layout)'

5 candidate functions:
 • 'Store(subgroup_matrix<K, S, C, R>  ✓ , ptr<workgroup, array<AE, AC>, read_write>  ✗ , offset: u32  ✓ , stride: u32  ✓ , matrix_layout  ✓ )' where:
      ✓  'S' is 'f32', 'i32', 'u32', 'f16', 'i8' or 'u8'
      ✗  'AE' is 'u16' or 'u32'
 • 'Store(subgroup_matrix<K, S, C, R>  ✓ , ptr<workgroup, array<vecN<AE>, AC>, read_write>  ✗ , offset: u32  ✓ , stride: u32  ✓ , matrix_layout  ✓ )' where:
      ✓  'S' is 'f32', 'i32', 'u32', 'f16', 'i8' or 'u8'
      ✗  'AE' is 'u16' or 'u32'
 • 'Store(subgroup_matrix<K, S, C, R>  ✓ , ptr<workgroup, array<S, AC>, read_write>  ✗ , offset: u32  ✓ , stride: u32  ✓ , matrix_layout  ✓ )' where:
      ✓  'S' is 'f32', 'i32', 'u32', 'f16', 'i8' or 'u8'
 • 'Store(subgroup_matrix<K, S, C, R>  ✓ , byte_address_buffer<AM>  ✗ , offset: u32  ✓ , stride: u32  ✓ , matrix_layout  ✓ )' where:
      ✓  'S' is 'f32', 'i32', 'u32', 'f16', 'i8' or 'u8'
      ✗  'AM' is 'write' or 'read_write'
 • 'Store(byte_address_buffer<write' or 'read_write>  ✗ , offset: u32  ✗ , value: u32  ✓ )'

    %54:void = %m.Store %out2, %53, 16u, 0u
                  ^^^^^

:15:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
main_inputs = struct @align(4) {
  tint_local_index:u32 @offset(0), @builtin(local_invocation_index)
}

$B1: {  # root
  %out0:ptr<workgroup, array<u32, 1024>, read_write> = var undef
  %out1:ptr<workgroup, array<vec2<u32>, 1024>, read_write> = var undef
  %out2:ptr<workgroup, array<vec3<f32>, 1024>, read_write> = var undef
  %out3:ptr<workgroup, array<vec4<u32>, 1024>, read_write> = var undef
  %out5:ptr<workgroup, array<u16, 2048>, read_write> = var undef
  %out6:ptr<workgroup, array<u16, 4096>, read_write> = var undef
}

%main_inner = func(%tint_local_index:u32):void {
  $B2: {
    loop [i: $B3, b: $B4, c: $B5] {  # loop_1
      $B3: {  # initializer
        next_iteration %tint_local_index  # -> $B4
      }
      $B4 (%idx:u32): {  # body
        %10:bool = gte %idx, 1024u
        if %10 [t: $B6] {  # if_1
          $B6: {  # true
            exit_loop  # loop_1
          }
        }
        %11:u32 = mul %idx, 4u
        %12:u32 = div %11, 4u
        %13:ptr<workgroup, u32, read_write> = access %out0, %12
        store %13, 0u
        %14:u32 = mul %idx, 8u
        %15:u32 = div %14, 8u
        %16:vec2<i32> = construct vec2<i32>(0i)
        %17:vec2<u32> = hlsl.asuint %16
        %18:ptr<workgroup, vec2<u32>, read_write> = access %out1, %15
        store %18, %17
        %19:ptr<workgroup, vec3<f32>, read_write> = access %out2, %idx
        store %19, vec3<f32>(0.0f)
        %20:u32 = mul %idx, 16u
        %21:u32 = div %20, 16u
        %22:ptr<workgroup, vec4<u32>, read_write> = access %out3, %21
        store %22, vec4<u32>(0u)
        %23:u32 = mul %idx, 4u
        %24:u32 = div %23, 2u
        %25:f16 = access vec2<f16>(0.0h), 0u
        %26:u16 = hlsl.asuint16 %25
        %27:ptr<workgroup, u16, read_write> = access %out5, %24
        store %27, %26
        %28:u32 = add %24, 1u
        %29:f16 = access vec2<f16>(0.0h), 1u
        %30:u16 = hlsl.asuint16 %29
        %31:ptr<workgroup, u16, read_write> = access %out5, %28
        store %31, %30
        %32:u32 = mul %idx, 8u
        %33:u32 = div %32, 2u
        %34:f16 = access vec3<f16>(0.0h), 0u
        %35:u16 = hlsl.asuint16 %34
        %36:ptr<workgroup, u16, read_write> = access %out6, %33
        store %36, %35
        %37:u32 = add %33, 1u
        %38:f16 = access vec3<f16>(0.0h), 1u
        %39:u16 = hlsl.asuint16 %38
        %40:ptr<workgroup, u16, read_write> = access %out6, %37
        store %40, %39
        %41:u32 = add %37, 1u
        %42:f16 = access vec3<f16>(0.0h), 2u
        %43:u16 = hlsl.asuint16 %42
        %44:ptr<workgroup, u16, read_write> = access %out6, %41
        store %44, %43
        continue  # -> $B5
      }
      $B5: {  # continuing
        %45:u32 = add %idx, 64u
        next_iteration %45  # -> $B4
      }
    }
    %46:void = workgroupBarrier
    %47:subgroup_matrix_result<f32, 8, 8> = construct
    %m:subgroup_matrix_result<f32, 8, 8> = let %47
    %49:u32 = hlsl.asuint 0i
    %50:void = %m.Store %out0, %49, 16u, 0u
    %51:u32 = hlsl.asuint 0i
    %52:void = %m.Store %out1, %51, 16u, 0u
    %53:u32 = hlsl.asuint 0i
    %54:void = %m.Store %out2, %53, 16u, 0u
    %55:u32 = hlsl.asuint 0i
    %56:void = %m.Store %out3, %55, 16u, 0u
    %57:u32 = div 0u, 2u
    %58:u32 = hlsl.asuint 0i
    %59:u32 = mul %58, 4u
    %60:u32 = div %59, 2u
    %61:u32 = add %57, %60
    %62:u32 = mul 16u, 4u
    %63:u32 = div %62, 2u
    %64:void = %m.Store %out5, %61, %63, 0u
    %65:u32 = div 0u, 2u
    %66:u32 = hlsl.asuint 0i
    %67:u32 = mul %66, 8u
    %68:u32 = div %67, 2u
    %69:u32 = add %65, %68
    %70:u32 = mul 16u, 8u
    %71:u32 = div %70, 2u
    %72:void = %m.Store %out6, %69, %71, 0u
    ret
  }
}
%main = @compute @workgroup_size(64i, 1i, 1i) func(%inputs:main_inputs):void {
  $B7: {
    %75:u32 = access %inputs, 0u
    %76:void = call %main_inner, %75
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
