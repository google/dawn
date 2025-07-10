SKIP: FAILED

Failed to generate: :18:14 error: glsl.textureSize: no matching call to 'glsl.textureSize(texel_buffer<r8unorm, read>, i32)'

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

:16:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  prevent_dce:u32 @offset(16), @location(0), @interpolate(flat)
}

prevent_dce_block = struct @align(4), @block {
  inner:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, prevent_dce_block, read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, texel_buffer<r8unorm, read>, read> = combined_texture_sampler undef @binding_point(0, 1)
}

%textureDimensions_afa26d = func():u32 {
  $B2: {
    %4:texel_buffer<r8unorm, read> = load %arg_0
    %5:i32 = glsl.textureSize %4, 0i
    %6:u32 = bitcast %5
    %res:ptr<function, u32, read_write> = var %6
    %8:u32 = load %res
    ret %8
  }
}
%fragment_main = @fragment func():void {
  $B3: {
    %10:u32 = call %textureDimensions_afa26d
    %11:ptr<storage, u32, read_write> = access %1, 0u
    store %11, %10
    ret
  }
}

Failed to generate: :18:14 error: glsl.textureSize: no matching call to 'glsl.textureSize(texel_buffer<r8unorm, read>, i32)'

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

:16:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  prevent_dce:u32 @offset(16), @location(0), @interpolate(flat)
}

prevent_dce_block = struct @align(4), @block {
  inner:u32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, prevent_dce_block, read_write> = var undef @binding_point(0, 0)
  %arg_0:ptr<handle, texel_buffer<r8unorm, read>, read> = combined_texture_sampler undef @binding_point(0, 1)
}

%textureDimensions_afa26d = func():u32 {
  $B2: {
    %4:texel_buffer<r8unorm, read> = load %arg_0
    %5:i32 = glsl.textureSize %4, 0i
    %6:u32 = bitcast %5
    %res:ptr<function, u32, read_write> = var %6
    %8:u32 = load %res
    ret %8
  }
}
%compute_main = @compute @workgroup_size(1i, 1i, 1i) func():void {
  $B3: {
    %10:u32 = call %textureDimensions_afa26d
    %11:ptr<storage, u32, read_write> = access %1, 0u
    store %11, %10
    ret
  }
}

Failed to generate: :13:14 error: glsl.textureSize: no matching call to 'glsl.textureSize(texel_buffer<r8unorm, read>, i32)'

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

    %4:i32 = glsl.textureSize %3, 0i
             ^^^^^^^^^^^^^^^^

:11:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  prevent_dce:u32 @offset(16), @location(0), @interpolate(flat)
}

$B1: {  # root
  %arg_0:ptr<handle, texel_buffer<r8unorm, read>, read> = combined_texture_sampler undef @binding_point(0, 0)
}

%textureDimensions_afa26d = func():u32 {
  $B2: {
    %3:texel_buffer<r8unorm, read> = load %arg_0
    %4:i32 = glsl.textureSize %3, 0i
    %5:u32 = bitcast %4
    %res:ptr<function, u32, read_write> = var %5
    %7:u32 = load %res
    ret %7
  }
}
%vertex_main = @vertex func():VertexOutput {
  $B3: {
    %out:ptr<function, VertexOutput, read_write> = var undef
    %10:ptr<function, vec4<f32>, read_write> = access %out, 0u
    store %10, vec4<f32>(0.0f)
    %11:ptr<function, u32, read_write> = access %out, 1u
    %12:u32 = call %textureDimensions_afa26d
    store %11, %12
    %13:VertexOutput = load %out
    ret %13
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
