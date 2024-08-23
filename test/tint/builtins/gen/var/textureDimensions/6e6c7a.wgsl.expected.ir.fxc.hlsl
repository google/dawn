SKIP: FAILED


@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@group(1) @binding(0) var arg_0 : texture_3d<u32>;

fn textureDimensions_6e6c7a() -> vec3<u32> {
  var arg_1 = 1u;
  var res : vec3<u32> = textureDimensions(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_6e6c7a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_6e6c7a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec3<u32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var tint_symbol : VertexOutput;
  tint_symbol.pos = vec4<f32>();
  tint_symbol.prevent_dce = textureDimensions_6e6c7a();
  return tint_symbol;
}

Failed to generate: :30:49 error: var: initializer type 'vec2<u32>' does not match store type 'vec3<u32>'
    %res:ptr<function, vec3<u32>, read_write> = var, %15
                                                ^^^

:17:3 note: in block
  $B2: {
  ^^^

note: # Disassembly
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0)
  prevent_dce:vec3<u32> @offset(16)
}

vertex_main_outputs = struct @align(16) {
  VertexOutput_prevent_dce:vec3<u32> @offset(0), @location(0), @interpolate(flat)
  VertexOutput_pos:vec4<f32> @offset(16), @builtin(position)
}

$B1: {  # root
  %prevent_dce:hlsl.byte_address_buffer<read_write> = var @binding_point(0, 0)
  %arg_0:ptr<handle, texture_3d<u32>, read> = var @binding_point(1, 0)
}

%textureDimensions_6e6c7a = func():vec3<u32> {
  $B2: {
    %arg_1:ptr<function, u32, read_write> = var, 1u
    %5:texture_3d<u32> = load %arg_0
    %6:u32 = load %arg_1
    %7:u32 = convert %6
    %8:ptr<function, vec4<u32>, read_write> = var
    %9:ptr<function, u32, read_write> = access %8, 0u
    %10:ptr<function, u32, read_write> = access %8, 1u
    %11:ptr<function, u32, read_write> = access %8, 2u
    %12:ptr<function, u32, read_write> = access %8, 3u
    %13:void = %5.GetDimensions %7, %9, %10, %11, %12
    %14:vec4<u32> = load %8
    %15:vec2<u32> = swizzle %14, xyz
    %res:ptr<function, vec3<u32>, read_write> = var, %15
    %17:vec3<u32> = load %res
    ret %17
  }
}
%fragment_main = @fragment func():void {
  $B3: {
    %19:vec3<u32> = call %textureDimensions_6e6c7a
    %20:void = %prevent_dce.Store3 0u, %19
    ret
  }
}
%compute_main = @compute @workgroup_size(1, 1, 1) func():void {
  $B4: {
    %22:vec3<u32> = call %textureDimensions_6e6c7a
    %23:void = %prevent_dce.Store3 0u, %22
    ret
  }
}
%vertex_main_inner = func():VertexOutput {
  $B5: {
    %tint_symbol:ptr<function, VertexOutput, read_write> = var
    %26:ptr<function, vec4<f32>, read_write> = access %tint_symbol, 0u
    store %26, vec4<f32>(0.0f)
    %27:ptr<function, vec3<u32>, read_write> = access %tint_symbol, 1u
    %28:vec3<u32> = call %textureDimensions_6e6c7a
    store %27, %28
    %29:VertexOutput = load %tint_symbol
    ret %29
  }
}
%vertex_main = @vertex func():vertex_main_outputs {
  $B6: {
    %31:VertexOutput = call %vertex_main_inner
    %32:vec4<f32> = access %31, 0u
    %33:vec3<u32> = access %31, 1u
    %34:vertex_main_outputs = construct %33, %32
    ret %34
  }
}


tint executable returned error: exit status 1
