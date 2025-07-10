SKIP: FAILED

VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0)
  prevent_dce:u32 @offset(16)
}

$B1: {  # root
  %prevent_dce:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, texel_buffer<rg16unorm, read>, read> = var undef @binding_point(1, 0)
}

%textureDimensions_7b7e31 = func():u32 {
  $B2: {
    %4:texel_buffer<rg16unorm, read> = load %arg_0
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
    %11:u32 = call %textureDimensions_7b7e31
    %12:void = %prevent_dce.Store 0u, %11
    ret
  }
}
Failed to generate: :15:18 error: GetDimensions: no matching call to 'GetDimensions(texel_buffer<rg16unorm, read>, ptr<function, u32, read_write>)'

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

:12:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0)
  prevent_dce:u32 @offset(16)
}

$B1: {  # root
  %prevent_dce:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, texel_buffer<rg16unorm, read>, read> = var undef @binding_point(1, 0)
}

%textureDimensions_7b7e31 = func():u32 {
  $B2: {
    %4:texel_buffer<rg16unorm, read> = load %arg_0
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
    %11:u32 = call %textureDimensions_7b7e31
    %12:void = %prevent_dce.Store 0u, %11
    ret
  }
}

VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0)
  prevent_dce:u32 @offset(16)
}

$B1: {  # root
  %prevent_dce:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, texel_buffer<rg16unorm, read>, read> = var undef @binding_point(1, 0)
}

%textureDimensions_7b7e31 = func():u32 {
  $B2: {
    %4:texel_buffer<rg16unorm, read> = load %arg_0
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
    %11:u32 = call %textureDimensions_7b7e31
    %12:void = %prevent_dce.Store 0u, %11
    ret
  }
}
Failed to generate: :15:18 error: GetDimensions: no matching call to 'GetDimensions(texel_buffer<rg16unorm, read>, ptr<function, u32, read_write>)'

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

:12:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0)
  prevent_dce:u32 @offset(16)
}

$B1: {  # root
  %prevent_dce:hlsl.byte_address_buffer<read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, texel_buffer<rg16unorm, read>, read> = var undef @binding_point(1, 0)
}

%textureDimensions_7b7e31 = func():u32 {
  $B2: {
    %4:texel_buffer<rg16unorm, read> = load %arg_0
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
    %11:u32 = call %textureDimensions_7b7e31
    %12:void = %prevent_dce.Store 0u, %11
    ret
  }
}

VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0)
  prevent_dce:u32 @offset(16)
}

vertex_main_outputs = struct @align(16) {
  VertexOutput_prevent_dce:u32 @offset(0), @location(0), @interpolate(flat)
  VertexOutput_pos:vec4<f32> @offset(16), @builtin(position)
}

$B1: {  # root
  %arg_0:ptr<handle, texel_buffer<rg16unorm, read>, read> = var undef @binding_point(1, 0)
}

%textureDimensions_7b7e31 = func():u32 {
  $B2: {
    %3:texel_buffer<rg16unorm, read> = load %arg_0
    %4:ptr<function, u32, read_write> = var undef
    %5:void = %3.GetDimensions %4
    %6:u32 = load %4
    %res:ptr<function, u32, read_write> = var %6
    %8:u32 = load %res
    ret %8
  }
}
%vertex_main_inner = func():VertexOutput {
  $B3: {
    %out:ptr<function, VertexOutput, read_write> = var undef
    %11:ptr<function, vec4<f32>, read_write> = access %out, 0u
    store %11, vec4<f32>(0.0f)
    %12:ptr<function, u32, read_write> = access %out, 1u
    %13:u32 = call %textureDimensions_7b7e31
    store %12, %13
    %14:VertexOutput = load %out
    ret %14
  }
}
%vertex_main = @vertex func():vertex_main_outputs {
  $B4: {
    %16:VertexOutput = call %vertex_main_inner
    %17:vec4<f32> = access %16, 0u
    %18:u32 = access %16, 1u
    %19:vertex_main_outputs = construct %18, %17
    ret %19
  }
}
Failed to generate: :19:18 error: GetDimensions: no matching call to 'GetDimensions(texel_buffer<rg16unorm, read>, ptr<function, u32, read_write>)'

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

    %5:void = %3.GetDimensions %4
                 ^^^^^^^^^^^^^

:16:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0)
  prevent_dce:u32 @offset(16)
}

vertex_main_outputs = struct @align(16) {
  VertexOutput_prevent_dce:u32 @offset(0), @location(0), @interpolate(flat)
  VertexOutput_pos:vec4<f32> @offset(16), @builtin(position)
}

$B1: {  # root
  %arg_0:ptr<handle, texel_buffer<rg16unorm, read>, read> = var undef @binding_point(1, 0)
}

%textureDimensions_7b7e31 = func():u32 {
  $B2: {
    %3:texel_buffer<rg16unorm, read> = load %arg_0
    %4:ptr<function, u32, read_write> = var undef
    %5:void = %3.GetDimensions %4
    %6:u32 = load %4
    %res:ptr<function, u32, read_write> = var %6
    %8:u32 = load %res
    ret %8
  }
}
%vertex_main_inner = func():VertexOutput {
  $B3: {
    %out:ptr<function, VertexOutput, read_write> = var undef
    %11:ptr<function, vec4<f32>, read_write> = access %out, 0u
    store %11, vec4<f32>(0.0f)
    %12:ptr<function, u32, read_write> = access %out, 1u
    %13:u32 = call %textureDimensions_7b7e31
    store %12, %13
    %14:VertexOutput = load %out
    ret %14
  }
}
%vertex_main = @vertex func():vertex_main_outputs {
  $B4: {
    %16:VertexOutput = call %vertex_main_inner
    %17:vec4<f32> = access %16, 0u
    %18:u32 = access %16, 1u
    %19:vertex_main_outputs = construct %18, %17
    ret %19
  }
}

//
// fragment_main
//
//
// compute_main
//
//
// vertex_main
//

tint executable returned error: exit status 1
