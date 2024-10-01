SKIP: FAILED


@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

var<private> tint_symbol_1 = vec4f();

fn textureLoad_6273b1() {
  var res = 0.0f;
  res = vec4f(textureLoad(arg_0, vec2i(), 1i), 0.0f, 0.0f, 0.0f).x;
  return;
}

fn tint_symbol_2(tint_symbol : vec4f) {
  tint_symbol_1 = tint_symbol;
  return;
}

fn vertex_main_1() {
  textureLoad_6273b1();
  tint_symbol_2(vec4f());
  return;
}

struct vertex_main_out {
  @builtin(position)
  tint_symbol_1_1 : vec4f,
}

@vertex
fn vertex_main() -> vertex_main_out {
  vertex_main_1();
  return vertex_main_out(tint_symbol_1);
}

Failed to generate: :16:14 error: glsl.texelFetch: no matching call to 'glsl.texelFetch(texture_depth_multisampled_2d, vec2<i32>, i32)'

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

    %8:f32 = glsl.texelFetch %5, %6, %7
             ^^^^^^^^^^^^^^^

:11:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
vertex_main_out = struct @align(16) {
  tint_symbol_1_1:vec4<f32> @offset(0), @builtin(position)
}

$B1: {  # root
  %arg_0:ptr<handle, texture_depth_multisampled_2d, read> = var @binding_point(0, 0)
  %tint_symbol_1:ptr<private, vec4<f32>, read_write> = var, vec4<f32>(0.0f)
}

%textureLoad_6273b1 = func():void {
  $B2: {
    %res:ptr<function, f32, read_write> = var, 0.0f
    %5:texture_depth_multisampled_2d = load %arg_0
    %6:vec2<i32> = convert vec2<i32>(0i)
    %7:i32 = convert 1i
    %8:f32 = glsl.texelFetch %5, %6, %7
    %9:vec4<f32> = construct %8, 0.0f, 0.0f, 0.0f
    %10:f32 = access %9, 0u
    store %res, %10
    ret
  }
}
%tint_symbol_2 = func(%tint_symbol:vec4<f32>):void {
  $B3: {
    store %tint_symbol_1, %tint_symbol
    ret
  }
}
%vertex_main_1 = func():void {
  $B4: {
    %14:void = call %textureLoad_6273b1
    %15:void = call %tint_symbol_2, vec4<f32>(0.0f)
    ret
  }
}
%vertex_main = @vertex func():vertex_main_out {
  $B5: {
    %17:void = call %vertex_main_1
    %18:vec4<f32> = load %tint_symbol_1
    %19:vertex_main_out = construct %18
    ret %19
  }
}


@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

fn textureLoad_6273b1() {
  var res = 0.0f;
  res = vec4f(textureLoad(arg_0, vec2i(), 1i), 0.0f, 0.0f, 0.0f).x;
  return;
}

struct vertex_main_out {
  @builtin(position)
  tint_symbol_1_1 : vec4f,
}

fn fragment_main_1() {
  textureLoad_6273b1();
  return;
}

@fragment
fn fragment_main() {
  fragment_main_1();
}

Failed to generate: :11:14 error: glsl.texelFetch: no matching call to 'glsl.texelFetch(texture_depth_multisampled_2d, vec2<i32>, i32)'

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

    %7:f32 = glsl.texelFetch %4, %5, %6
             ^^^^^^^^^^^^^^^

:6:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %arg_0:ptr<handle, texture_depth_multisampled_2d, read> = var @binding_point(0, 0)
}

%textureLoad_6273b1 = func():void {
  $B2: {
    %res:ptr<function, f32, read_write> = var, 0.0f
    %4:texture_depth_multisampled_2d = load %arg_0
    %5:vec2<i32> = convert vec2<i32>(0i)
    %6:i32 = convert 1i
    %7:f32 = glsl.texelFetch %4, %5, %6
    %8:vec4<f32> = construct %7, 0.0f, 0.0f, 0.0f
    %9:f32 = access %8, 0u
    store %res, %9
    ret
  }
}
%fragment_main_1 = func():void {
  $B3: {
    %11:void = call %textureLoad_6273b1
    ret
  }
}
%fragment_main = @fragment func():void {
  $B4: {
    %13:void = call %fragment_main_1
    ret
  }
}


@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

fn textureLoad_6273b1() {
  var res = 0.0f;
  res = vec4f(textureLoad(arg_0, vec2i(), 1i), 0.0f, 0.0f, 0.0f).x;
  return;
}

struct vertex_main_out {
  @builtin(position)
  tint_symbol_1_1 : vec4f,
}

fn compute_main_1() {
  textureLoad_6273b1();
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main() {
  compute_main_1();
}

Failed to generate: :11:14 error: glsl.texelFetch: no matching call to 'glsl.texelFetch(texture_depth_multisampled_2d, vec2<i32>, i32)'

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

    %7:f32 = glsl.texelFetch %4, %5, %6
             ^^^^^^^^^^^^^^^

:6:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
$B1: {  # root
  %arg_0:ptr<handle, texture_depth_multisampled_2d, read> = var @binding_point(0, 0)
}

%textureLoad_6273b1 = func():void {
  $B2: {
    %res:ptr<function, f32, read_write> = var, 0.0f
    %4:texture_depth_multisampled_2d = load %arg_0
    %5:vec2<i32> = convert vec2<i32>(0i)
    %6:i32 = convert 1i
    %7:f32 = glsl.texelFetch %4, %5, %6
    %8:vec4<f32> = construct %7, 0.0f, 0.0f, 0.0f
    %9:f32 = access %8, 0u
    store %res, %9
    ret
  }
}
%compute_main_1 = func():void {
  $B3: {
    %11:void = call %textureLoad_6273b1
    ret
  }
}
%compute_main = @compute @workgroup_size(1, 1, 1) func():void {
  $B4: {
    %13:void = call %compute_main_1
    ret
  }
}


tint executable returned error: exit status 1
