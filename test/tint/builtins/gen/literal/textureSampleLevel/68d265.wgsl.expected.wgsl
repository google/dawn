@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@group(1) @binding(0) var arg_0 : texture_1d<f32, unfilterable>;

@group(1) @binding(1) var arg_1 : sampler<non_filtering>;

fn textureSampleLevel_68d265() -> vec4<f32> {
  var res : vec4<f32> = textureSampleLevel(arg_0, arg_1, 1.0f, 1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = textureSampleLevel_68d265();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = textureSampleLevel_68d265();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = textureSampleLevel_68d265();
  return out;
}
