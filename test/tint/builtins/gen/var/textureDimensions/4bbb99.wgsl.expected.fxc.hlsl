SKIP: FAILED

$B1: {  # root
  %prevent_dce:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, texel_buffer<rg8unorm, read_write>, read> = var undef @binding_point(1, 0)
}

%textureDimensions_4bbb99 = func():u32 {
  $B2: {
    %4:texel_buffer<rg8unorm, read_write> = load %arg_0
    %5:ptr<function, u32, read_write> = var undef
    %6:void = %4.GetDimensions %5
    %7:u32 = load %5
    %res:ptr<function, u32, read_write> = var %7
    %9:u32 = load %res
    ret %9
  }
}
%fragment_main = @fragment func():void {
  $B3: {
    %11:u32 = call %textureDimensions_4bbb99
    %12:void = %prevent_dce.Store 0u, %11
    ret
  }
}
Failed to generate: :10:18 error: GetDimensions: no matching call to 'GetDimensions(texel_buffer<rg8unorm, read_write>, ptr<function, u32, read_write>)'

27 candidate functions:
 • 'GetDimensions(byte_address_buffer<A>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ )'
 • 'GetDimensions(texture: texture_storage_1d<F, A>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ )'
 • 'GetDimensions(texture: texture_1d<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_storage_2d<F, A>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_2d  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_cube  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_2d<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_cube<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_storage_2d_array<F, A>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_storage_3d<F, A>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , depth: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_2d_array  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_cube_array  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_multisampled_2d  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , samples: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_2d_array<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_3d<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , depth: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_cube_array<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_multisampled_2d<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , samples: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_1d<T>  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_depth_2d  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_cube  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_2d<T>  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_cube<T>  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_depth_2d_array  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_cube_array  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_2d_array<T>  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_3d<T>  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , depth: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_cube_array<T>  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'

    %6:void = %4.GetDimensions %5
                 ^^^^^^^^^^^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %prevent_dce:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, texel_buffer<rg8unorm, read_write>, read> = var undef @binding_point(1, 0)
}

%textureDimensions_4bbb99 = func():u32 {
  $B2: {
    %4:texel_buffer<rg8unorm, read_write> = load %arg_0
    %5:ptr<function, u32, read_write> = var undef
    %6:void = %4.GetDimensions %5
    %7:u32 = load %5
    %res:ptr<function, u32, read_write> = var %7
    %9:u32 = load %res
    ret %9
  }
}
%fragment_main = @fragment func():void {
  $B3: {
    %11:u32 = call %textureDimensions_4bbb99
    %12:void = %prevent_dce.Store 0u, %11
    ret
  }
}

$B1: {  # root
  %prevent_dce:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, texel_buffer<rg8unorm, read_write>, read> = var undef @binding_point(1, 0)
}

%textureDimensions_4bbb99 = func():u32 {
  $B2: {
    %4:texel_buffer<rg8unorm, read_write> = load %arg_0
    %5:ptr<function, u32, read_write> = var undef
    %6:void = %4.GetDimensions %5
    %7:u32 = load %5
    %res:ptr<function, u32, read_write> = var %7
    %9:u32 = load %res
    ret %9
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %11:u32 = call %textureDimensions_4bbb99
    %12:void = %prevent_dce.Store 0u, %11
    ret
  }
}
Failed to generate: :10:18 error: GetDimensions: no matching call to 'GetDimensions(texel_buffer<rg8unorm, read_write>, ptr<function, u32, read_write>)'

27 candidate functions:
 • 'GetDimensions(byte_address_buffer<A>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ )'
 • 'GetDimensions(texture: texture_storage_1d<F, A>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ )'
 • 'GetDimensions(texture: texture_1d<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_storage_2d<F, A>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_2d  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_cube  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_2d<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_cube<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_storage_2d_array<F, A>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_storage_3d<F, A>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , depth: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_2d_array  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_cube_array  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_multisampled_2d  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , samples: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_2d_array<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_3d<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , depth: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_cube_array<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_multisampled_2d<T>  ✗ , width: ptr<function, u32, write' or 'read_write>  ✓ , height: ptr<function, u32, write' or 'read_write>  ✗ , samples: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_1d<T>  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_depth_2d  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_cube  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_2d<T>  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_cube<T>  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_depth_2d_array  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_depth_cube_array  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )'
 • 'GetDimensions(texture: texture_2d_array<T>  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_3d<T>  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , depth: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'GetDimensions(texture: texture_cube_array<T>  ✗ , level: u32  ✗ , width: ptr<function, u32, write' or 'read_write>  ✗ , height: ptr<function, u32, write' or 'read_write>  ✗ , elements: ptr<function, u32, write' or 'read_write>  ✗ , num_levels: ptr<function, u32, write' or 'read_write>  ✗ )' where:
      ✗  'T' is 'f32', 'i32' or 'u32'

    %6:void = %4.GetDimensions %5
                 ^^^^^^^^^^^^^

:7:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %prevent_dce:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, texel_buffer<rg8unorm, read_write>, read> = var undef @binding_point(1, 0)
}

%textureDimensions_4bbb99 = func():u32 {
  $B2: {
    %4:texel_buffer<rg8unorm, read_write> = load %arg_0
    %5:ptr<function, u32, read_write> = var undef
    %6:void = %4.GetDimensions %5
    %7:u32 = load %5
    %res:ptr<function, u32, read_write> = var %7
    %9:u32 = load %res
    ret %9
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %11:u32 = call %textureDimensions_4bbb99
    %12:void = %prevent_dce.Store 0u, %11
    ret
  }
}

//
// fragment_main
//
//
// compute_main
//

tint executable returned error: exit status 1
