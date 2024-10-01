SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

fn textureLoad_4db25c() -> f32 {
  var res : f32 = textureLoad(arg_0, vec2<u32>(1u), 1u);
  return res;
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f32,
}

@fragment
fn fragment_main() {
  prevent_dce = textureLoad_4db25c();
}

Failed to generate: :15:14 error: glsl.texelFetch: no matching call to 'glsl.texelFetch(texture_depth_multisampled_2d, vec2<i32>, i32)'

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

:11:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
tint_symbol_1 = struct @align(4), @block {
  tint_symbol:f32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read_write> = var @binding_point(0, 0)
  %arg_0:ptr<handle, texture_depth_multisampled_2d, read> = var @binding_point(0, 0)
}

%textureLoad_4db25c = func():f32 {
  $B2: {
    %4:texture_depth_multisampled_2d = load %arg_0
    %5:vec2<i32> = convert vec2<u32>(1u)
    %6:i32 = convert 1u
    %7:f32 = glsl.texelFetch %4, %5, %6
    %res:ptr<function, f32, read_write> = var, %7
    %9:f32 = load %res
    ret %9
  }
}
%fragment_main = @fragment func():void {
  $B3: {
    %11:f32 = call %textureLoad_4db25c
    %12:ptr<storage, f32, read_write> = access %1, 0u
    store %12, %11
    ret
  }
}


@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

fn textureLoad_4db25c() -> f32 {
  var res : f32 = textureLoad(arg_0, vec2<u32>(1u), 1u);
  return res;
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f32,
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureLoad_4db25c();
}

Failed to generate: :15:14 error: glsl.texelFetch: no matching call to 'glsl.texelFetch(texture_depth_multisampled_2d, vec2<i32>, i32)'

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

:11:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
tint_symbol_1 = struct @align(4), @block {
  tint_symbol:f32 @offset(0)
}

$B1: {  # root
  %1:ptr<storage, tint_symbol_1, read_write> = var @binding_point(0, 0)
  %arg_0:ptr<handle, texture_depth_multisampled_2d, read> = var @binding_point(0, 0)
}

%textureLoad_4db25c = func():f32 {
  $B2: {
    %4:texture_depth_multisampled_2d = load %arg_0
    %5:vec2<i32> = convert vec2<u32>(1u)
    %6:i32 = convert 1u
    %7:f32 = glsl.texelFetch %4, %5, %6
    %res:ptr<function, f32, read_write> = var, %7
    %9:f32 = load %res
    ret %9
  }
}
%compute_main = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %11:f32 = call %textureLoad_4db25c
    %12:ptr<storage, f32, read_write> = access %1, 0u
    store %12, %11
    ret
  }
}


@group(1) @binding(0) var arg_0 : texture_depth_multisampled_2d;

fn textureLoad_4db25c() -> f32 {
  var res : f32 = textureLoad(arg_0, vec2<u32>(1u), 1u);
  return res;
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var tint_symbol : VertexOutput;
  tint_symbol.pos = vec4<f32>();
  tint_symbol.prevent_dce = textureLoad_4db25c();
  return tint_symbol;
}

Failed to generate: :15:14 error: glsl.texelFetch: no matching call to 'glsl.texelFetch(texture_depth_multisampled_2d, vec2<i32>, i32)'

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

    %6:f32 = glsl.texelFetch %3, %4, %5
             ^^^^^^^^^^^^^^^

:11:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
  prevent_dce:f32 @offset(16), @location(0), @interpolate(flat)
}

$B1: {  # root
  %arg_0:ptr<handle, texture_depth_multisampled_2d, read> = var @binding_point(0, 0)
}

%textureLoad_4db25c = func():f32 {
  $B2: {
    %3:texture_depth_multisampled_2d = load %arg_0
    %4:vec2<i32> = convert vec2<u32>(1u)
    %5:i32 = convert 1u
    %6:f32 = glsl.texelFetch %3, %4, %5
    %res:ptr<function, f32, read_write> = var, %6
    %8:f32 = load %res
    ret %8
  }
}
%vertex_main = @vertex func():VertexOutput {
  $B3: {
    %tint_symbol:ptr<function, VertexOutput, read_write> = var
    %11:ptr<function, vec4<f32>, read_write> = access %tint_symbol, 0u
    store %11, vec4<f32>(0.0f)
    %12:ptr<function, f32, read_write> = access %tint_symbol, 1u
    %13:f32 = call %textureLoad_4db25c
    store %12, %13
    %14:VertexOutput = load %tint_symbol
    ret %14
  }
}


tint executable returned error: exit status 1
