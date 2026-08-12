SKIP: FAILED

../../src/tint/lang/core/ir/validator.cc:111 internal compiler error: 
=========================================================
== IR validation failed before core.VectorizeScalarMatrixConstructors:
=========================================================
:85:19 error: Store: no matching call to 'Store(subgroup_matrix_right<f16, 8, 8>, ptr<workgroup, array<vec2<f16>, 1024>, read_write>, u32, u32, hlsl.matrix_layout)'

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

    %54:void = %m.Store %out5, 0u, 16u, 1u
                  ^^^^^

:16:3 note: in block
  $B2: {
  ^^^

:86:19 error: Store: no matching call to 'Store(subgroup_matrix_right<f16, 8, 8>, ptr<workgroup, array<vec3<f16>, 1024>, read_write>, u32, u32, hlsl.matrix_layout)'

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

    %55:void = %m.Store %out6, 0u, 16u, 1u
                  ^^^^^

:16:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
main_inputs = struct @align(4) {
  tint_local_index:u32 @offset(0), @builtin(local_invocation_index)
}

$B1: {  # root
  %out0:ptr<workgroup, array<u32, 1024>, read_write> = var undef
  %out1:ptr<workgroup, array<vec2<u32>, 1024>, read_write> = var undef
  %out2:ptr<workgroup, array<u32, 4096>, read_write> = var undef
  %out3:ptr<workgroup, array<vec4<u32>, 1024>, read_write> = var undef
  %out4:ptr<workgroup, array<f16, 1024>, read_write> = var undef
  %out5:ptr<workgroup, array<vec2<f16>, 1024>, read_write> = var undef
  %out6:ptr<workgroup, array<vec3<f16>, 1024>, read_write> = var undef
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
        %14:ptr<workgroup, u32, read_write> = access %out0, %13
        store %14, 0u
        %15:u32 = mul %idx, 8u
        %16:u32 = div %15, 8u
        %17:vec2<i32> = construct vec2<i32>(0i)
        %18:vec2<u32> = hlsl.asuint %17
        %19:ptr<workgroup, vec2<u32>, read_write> = access %out1, %16
        store %19, %18
        %20:u32 = mul %idx, 16u
        %21:u32 = div %20, 4u
        %22:f32 = access vec3<f32>(0.0f), 0u
        %23:u32 = hlsl.asuint %22
        %24:ptr<workgroup, u32, read_write> = access %out2, %21
        store %24, %23
        %25:u32 = add %21, 1u
        %26:f32 = access vec3<f32>(0.0f), 1u
        %27:u32 = hlsl.asuint %26
        %28:ptr<workgroup, u32, read_write> = access %out2, %25
        store %28, %27
        %29:u32 = add %25, 1u
        %30:f32 = access vec3<f32>(0.0f), 2u
        %31:u32 = hlsl.asuint %30
        %32:ptr<workgroup, u32, read_write> = access %out2, %29
        store %32, %31
        %33:u32 = mul %idx, 16u
        %34:u32 = div %33, 16u
        %35:ptr<workgroup, vec4<u32>, read_write> = access %out3, %34
        store %35, vec4<u32>(0u)
        %36:ptr<workgroup, f16, read_write> = access %out4, %idx
        store %36, 0.0h
        %37:ptr<workgroup, vec2<f16>, read_write> = access %out5, %idx
        store %37, vec2<f16>(0.0h)
        %38:ptr<workgroup, vec3<f16>, read_write> = access %out6, %idx
        store %38, vec3<f16>(0.0h)
        continue  # -> $B5
      }
      $B5: {  # continuing
        %39:u32 = add %idx, 64u
        next_iteration %39  # -> $B4
      }
    }
    %40:void = workgroupBarrier
    %41:subgroup_matrix_right<f16, 8, 8> = construct
    %m:subgroup_matrix_right<f16, 8, 8> = let %41
    %43:void = %m.Store %out0, 0u, 16u, 1u
    %44:void = %m.Store %out1, 0u, 16u, 1u
    %45:u32 = div 0u, 4u
    %46:u32 = mul 0u, 16u
    %47:u32 = div %46, 4u
    %48:u32 = add %45, %47
    %49:u32 = mul 16u, 16u
    %50:u32 = div %49, 4u
    %51:void = %m.Store %out2, %48, %50, 1u
    %52:void = %m.Store %out3, 0u, 16u, 1u
    %53:void = %m.Store %out4, 0u, 16u, 1u
    %54:void = %m.Store %out5, 0u, 16u, 1u
    %55:void = %m.Store %out6, 0u, 16u, 1u
    ret
  }
}
%main = @compute @workgroup_size(64i, 1i, 1i) func(%inputs:main_inputs):void {
  $B7: {
    %58:u32 = access %inputs, 0u
    %59:void = call %main_inner, %58
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
