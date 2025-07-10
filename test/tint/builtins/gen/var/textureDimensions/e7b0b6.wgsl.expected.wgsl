requires texel_buffers;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

@group(1) @binding(0) var arg_0 : texel_buffer<rgb10a2uint, read>;

fn textureDimensions_e7b0b6() -> u32 {
  var res : u32 = textureDimensions(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureDimensions_e7b0b6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureDimensions_e7b0b6();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : u32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureDimensions_e7b0b6();
  return out;
}
