SKIP: FAILED


@group(2) @binding(1) var x_20 : texture_depth_multisampled_2d;

fn main_1() {
  let f1 = 1.0f;
  let vf12 = vec2f(1.0f, 2.0f);
  let vf123 = vec3f(1.0f, 2.0f, 3.0f);
  let vf1234 = vec4f(1.0f, 2.0f, 3.0f, 4.0f);
  let vi123 = vec3i(1i, 2i, 3i);
  let vi1234 = vec4i(1i, 2i, 3i, 4i);
  let u1 = 1u;
  let vu12 = vec2u(1u, 2u);
  let vu123 = vec3u(1u, 2u, 3u);
  let vu1234 = vec4u(1u, 2u, 3u, 4u);
  let offsets2d = vec2i(3i, 4i);
  let x_99 = vec4f(textureLoad(x_20, vec2i(1i, 2i), 1i), 0.0f, 0.0f, 0.0f);
  return;
}

@fragment
fn tint_symbol() {
  main_1();
}

Failed to generate: :21:15 error: glsl.texelFetch: no matching call to 'glsl.texelFetch(texture_depth_multisampled_2d, vec2<i32>, i32)'

5 candidate functions:
 • 'glsl.texelFetch(texture: texture_2d<T>  ✗ , location: vec2<i32>  ✓ , level: i32  ✓ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.texelFetch(texture: texture_multisampled_2d<T>  ✗ , location: vec2<i32>  ✓ , sample_index: i32  ✓ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.texelFetch(texture: texture_1d<T>  ✗ , location: i32  ✗ , level: i32  ✓ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.texelFetch(texture: texture_2d_array<T>  ✗ , location: vec3<i32>  ✗ , level: i32  ✓ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'
 • 'glsl.texelFetch(texture: texture_3d<T>  ✗ , location: vec3<i32>  ✗ , level: i32  ✓ ) -> vec4<T>' where:
      ✗  'T' is 'f32', 'i32' or 'u32'

    %17:f32 = glsl.texelFetch %14, %15, %16
              ^^^^^^^^^^^^^^^

:6:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %x_20:ptr<handle, texture_depth_multisampled_2d, read> = var @binding_point(0, 1)
}

%main_1 = func():void {
  $B2: {
    %f1:f32 = let 1.0f
    %vf12:vec2<f32> = let vec2<f32>(1.0f, 2.0f)
    %vf123:vec3<f32> = let vec3<f32>(1.0f, 2.0f, 3.0f)
    %vf1234:vec4<f32> = let vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f)
    %vi123:vec3<i32> = let vec3<i32>(1i, 2i, 3i)
    %vi1234:vec4<i32> = let vec4<i32>(1i, 2i, 3i, 4i)
    %u1:u32 = let 1u
    %vu12:vec2<u32> = let vec2<u32>(1u, 2u)
    %vu123:vec3<u32> = let vec3<u32>(1u, 2u, 3u)
    %vu1234:vec4<u32> = let vec4<u32>(1u, 2u, 3u, 4u)
    %offsets2d:vec2<i32> = let vec2<i32>(3i, 4i)
    %14:texture_depth_multisampled_2d = load %x_20
    %15:vec2<i32> = convert vec2<i32>(1i, 2i)
    %16:i32 = convert 1i
    %17:f32 = glsl.texelFetch %14, %15, %16
    %18:vec4<f32> = construct %17, 0.0f, 0.0f, 0.0f
    %x_99:vec4<f32> = let %18
    ret
  }
}
%tint_symbol = @fragment func():void {
  $B3: {
    %21:void = call %main_1
    ret
  }
}


tint executable returned error: exit status 1
