SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

@group(1) @binding(0) var arg_0 : texture_1d<f32>;

fn textureDimensions_aac604() -> u32 {
  var res : u32 = textureDimensions(arg_0, 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_aac604();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_aac604();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : u32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var tint_symbol : VertexOutput;
  tint_symbol.pos = vec4<f32>();
  tint_symbol.prevent_dce = textureDimensions_aac604();
  return tint_symbol;
}

Failed to generate: :26:43 error: var: initializer type 'vec2<u32>' does not match store type 'u32'
    %res:ptr<function, u32, read_write> = var, %11
                                          ^^^

:17:3 note: in block
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
  %prevent_dce:hlsl.byte_address_buffer<read_write> = var @binding_point(0, 0)
  %arg_0:ptr<handle, texture_1d<f32>, read> = var @binding_point(1, 0)
}

%textureDimensions_aac604 = func():u32 {
  $B2: {
    %4:texture_1d<f32> = load %arg_0
    %5:u32 = convert 1u
    %6:ptr<function, vec2<u32>, read_write> = var
    %7:ptr<function, u32, read_write> = access %6, 0u
    %8:ptr<function, u32, read_write> = access %6, 1u
    %9:void = %4.GetDimensions %5, %7, %8
    %10:vec2<u32> = load %6
    %11:vec2<u32> = swizzle %10, x
    %res:ptr<function, u32, read_write> = var, %11
    %13:u32 = load %res
    ret %13
  }
}
%fragment_main = @fragment func():void {
  $B3: {
    %15:u32 = call %textureDimensions_aac604
    %16:void = %prevent_dce.Store 0u, %15
    ret
  }
}
%compute_main = @compute @workgroup_size(1, 1, 1) func():void {
  $B4: {
    %18:u32 = call %textureDimensions_aac604
    %19:void = %prevent_dce.Store 0u, %18
    ret
  }
}
%vertex_main_inner = func():VertexOutput {
  $B5: {
    %tint_symbol:ptr<function, VertexOutput, read_write> = var
    %22:ptr<function, vec4<f32>, read_write> = access %tint_symbol, 0u
    store %22, vec4<f32>(0.0f)
    %23:ptr<function, u32, read_write> = access %tint_symbol, 1u
    %24:u32 = call %textureDimensions_aac604
    store %23, %24
    %25:VertexOutput = load %tint_symbol
    ret %25
  }
}
%vertex_main = @vertex func():vertex_main_outputs {
  $B6: {
    %27:VertexOutput = call %vertex_main_inner
    %28:vec4<f32> = access %27, 0u
    %29:u32 = access %27, 1u
    %30:vertex_main_outputs = construct %29, %28
    ret %30
  }
}


tint executable returned error: exit status 1
