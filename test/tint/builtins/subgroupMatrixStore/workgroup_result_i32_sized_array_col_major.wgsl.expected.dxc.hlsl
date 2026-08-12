SKIP: FAILED

../../src/tint/lang/core/ir/validator.cc:111 internal compiler error: 
=========================================================
== IR validation failed before core.VectorizeScalarMatrixConstructors:
=========================================================
:91:19 error: Store: no matching call to 'Store(subgroup_matrix_result<i32, 8, 8>, ptr<workgroup, array<vec2<i32>, 1024>, read_write>, u32, u32, hlsl.matrix_layout)'

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

    %58:void = %m.Store %out1, 0u, 16u, 1u
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
  %out1:ptr<workgroup, array<vec2<i32>, 1024>, read_write> = var undef
  %out2:ptr<workgroup, array<u32, 4096>, read_write> = var undef
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
        %14:ptr<workgroup, vec2<i32>, read_write> = access %out1, %idx
        store %14, vec2<i32>(0i)
        %15:u32 = mul %idx, 16u
        %16:u32 = div %15, 4u
        %17:f32 = access vec3<f32>(0.0f), 0u
        %18:u32 = hlsl.asuint %17
        %19:ptr<workgroup, u32, read_write> = access %out2, %16
        store %19, %18
        %20:u32 = add %16, 1u
        %21:f32 = access vec3<f32>(0.0f), 1u
        %22:u32 = hlsl.asuint %21
        %23:ptr<workgroup, u32, read_write> = access %out2, %20
        store %23, %22
        %24:u32 = add %20, 1u
        %25:f32 = access vec3<f32>(0.0f), 2u
        %26:u32 = hlsl.asuint %25
        %27:ptr<workgroup, u32, read_write> = access %out2, %24
        store %27, %26
        %28:u32 = mul %idx, 16u
        %29:u32 = div %28, 16u
        %30:ptr<workgroup, vec4<u32>, read_write> = access %out3, %29
        store %30, vec4<u32>(0u)
        %31:u32 = mul %idx, 4u
        %32:u32 = div %31, 2u
        %33:f16 = access vec2<f16>(0.0h), 0u
        %34:u16 = hlsl.asuint16 %33
        %35:ptr<workgroup, u16, read_write> = access %out5, %32
        store %35, %34
        %36:u32 = add %32, 1u
        %37:f16 = access vec2<f16>(0.0h), 1u
        %38:u16 = hlsl.asuint16 %37
        %39:ptr<workgroup, u16, read_write> = access %out5, %36
        store %39, %38
        %40:u32 = mul %idx, 8u
        %41:u32 = div %40, 2u
        %42:f16 = access vec3<f16>(0.0h), 0u
        %43:u16 = hlsl.asuint16 %42
        %44:ptr<workgroup, u16, read_write> = access %out6, %41
        store %44, %43
        %45:u32 = add %41, 1u
        %46:f16 = access vec3<f16>(0.0h), 1u
        %47:u16 = hlsl.asuint16 %46
        %48:ptr<workgroup, u16, read_write> = access %out6, %45
        store %48, %47
        %49:u32 = add %45, 1u
        %50:f16 = access vec3<f16>(0.0h), 2u
        %51:u16 = hlsl.asuint16 %50
        %52:ptr<workgroup, u16, read_write> = access %out6, %49
        store %52, %51
        continue  # -> $B5
      }
      $B5: {  # continuing
        %53:u32 = add %idx, 64u
        next_iteration %53  # -> $B4
      }
    }
    %54:void = workgroupBarrier
    %55:subgroup_matrix_result<i32, 8, 8> = construct
    %m:subgroup_matrix_result<i32, 8, 8> = let %55
    %57:void = %m.Store %out0, 0u, 16u, 1u
    %58:void = %m.Store %out1, 0u, 16u, 1u
    %59:u32 = div 0u, 4u
    %60:u32 = mul 0u, 16u
    %61:u32 = div %60, 4u
    %62:u32 = add %59, %61
    %63:u32 = mul 16u, 16u
    %64:u32 = div %63, 4u
    %65:void = %m.Store %out2, %62, %64, 1u
    %66:void = %m.Store %out3, 0u, 16u, 1u
    %67:u32 = div 0u, 2u
    %68:u32 = mul 0u, 4u
    %69:u32 = div %68, 2u
    %70:u32 = add %67, %69
    %71:u32 = mul 16u, 4u
    %72:u32 = div %71, 2u
    %73:void = %m.Store %out5, %70, %72, 1u
    %74:u32 = div 0u, 2u
    %75:u32 = mul 0u, 8u
    %76:u32 = div %75, 2u
    %77:u32 = add %74, %76
    %78:u32 = mul 16u, 8u
    %79:u32 = div %78, 2u
    %80:void = %m.Store %out6, %77, %79, 1u
    ret
  }
}
%main = @compute @workgroup_size(64i, 1i, 1i) func(%inputs:main_inputs):void {
  $B7: {
    %83:u32 = access %inputs, 0u
    %84:void = call %main_inner, %83
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
