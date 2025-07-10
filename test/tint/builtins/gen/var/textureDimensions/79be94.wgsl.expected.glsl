SKIP: FAILED

Failed to generate: :13:14 error: glsl.textureSize: no matching call to 'glsl.textureSize(texel_buffer<r32sint, read_write>, i32)'

11 candidate functions:
 • 'glsl.textureSize(texture: texture_depth_2d  ✗ , level: i32  ✓ ) -> vec2<i32>'
 • 'glsl.textureSize(texture: texture_depth_2d_array  ✗ , level: i32  ✓ ) -> vec3<i32>'
 • 'glsl.textureSize(texture: texture_depth_cube  ✗ , level: i32  ✓ ) -> vec2<i32>'
 • 'glsl.textureSize(texture: texture_depth_cube_array  ✗ , level: i32  ✓ ) -> vec3<i32>'
 • 'glsl.textureSize(texture: texture_2d<T>  ✗ , level: i32  ✓ ) -> vec2<i32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.textureSize(texture: texture_2d_array<T>  ✗ , level: i32  ✓ ) -> vec3<i32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.textureSize(texture: texture_3d<T>  ✗ , level: i32  ✓ ) -> vec3<i32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.textureSize(texture: texture_cube<T>  ✗ , level: i32  ✓ ) -> vec2<i32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.textureSize(texture: texture_cube_array<T>  ✗ , level: i32  ✓ ) -> vec3<i32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.textureSize(texture: texture_depth_multisampled_2d  ✗ ) -> vec2<i32>'
 • 'glsl.textureSize(texture: texture_multisampled_2d<T>  ✗ ) -> vec2<i32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'

    %5:i32 = glsl.textureSize %4, 0i
             ^^^^^^^^^^^^^^^^

:11:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
prevent_dce_block = struct @align(4), @block {
  inner:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, prevent_dce_block, read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, texel_buffer<r32sint, read_write>, read> = combined_texture_sampler undef @binding_point(0, 1)
}

%textureDimensions_79be94 = func():u32 {
  $B2: {
    %4:texel_buffer<r32sint, read_write> = load %arg_0
    %5:i32 = glsl.textureSize %4, 0i
    %6:u32 = bitcast %5
    %res:ptr<function, u32, read_write> = var %6
    %8:u32 = load %res
    ret %8
  }
}
%fragment_main = @fragment func():void {
  $B3: {
    %10:u32 = call %textureDimensions_79be94
    %11:ptr<storage, u32, read_write> = access %1, 0u
    store %11, %10
    ret
  }
}

Failed to generate: :13:14 error: glsl.textureSize: no matching call to 'glsl.textureSize(texel_buffer<r32sint, read_write>, i32)'

11 candidate functions:
 • 'glsl.textureSize(texture: texture_depth_2d  ✗ , level: i32  ✓ ) -> vec2<i32>'
 • 'glsl.textureSize(texture: texture_depth_2d_array  ✗ , level: i32  ✓ ) -> vec3<i32>'
 • 'glsl.textureSize(texture: texture_depth_cube  ✗ , level: i32  ✓ ) -> vec2<i32>'
 • 'glsl.textureSize(texture: texture_depth_cube_array  ✗ , level: i32  ✓ ) -> vec3<i32>'
 • 'glsl.textureSize(texture: texture_2d<T>  ✗ , level: i32  ✓ ) -> vec2<i32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.textureSize(texture: texture_2d_array<T>  ✗ , level: i32  ✓ ) -> vec3<i32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.textureSize(texture: texture_3d<T>  ✗ , level: i32  ✓ ) -> vec3<i32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.textureSize(texture: texture_cube<T>  ✗ , level: i32  ✓ ) -> vec2<i32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.textureSize(texture: texture_cube_array<T>  ✗ , level: i32  ✓ ) -> vec3<i32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.textureSize(texture: texture_depth_multisampled_2d  ✗ ) -> vec2<i32>'
 • 'glsl.textureSize(texture: texture_multisampled_2d<T>  ✗ ) -> vec2<i32>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'

    %5:i32 = glsl.textureSize %4, 0i
             ^^^^^^^^^^^^^^^^

:11:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
prevent_dce_block = struct @align(4), @block {
  inner:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, prevent_dce_block, read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, texel_buffer<r32sint, read_write>, read> = combined_texture_sampler undef @binding_point(0, 1)
}

%textureDimensions_79be94 = func():u32 {
  $B2: {
    %4:texel_buffer<r32sint, read_write> = load %arg_0
    %5:i32 = glsl.textureSize %4, 0i
    %6:u32 = bitcast %5
    %res:ptr<function, u32, read_write> = var %6
    %8:u32 = load %res
    ret %8
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %10:u32 = call %textureDimensions_79be94
    %11:ptr<storage, u32, read_write> = access %1, 0u
    store %11, %10
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
