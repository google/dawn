@group(1) @binding(0) var arg_0 : texture_depth_2d_array;

@group(1) @binding(1) var arg_1 : sampler;

fn textureGather_2a4f40() {
  var res : vec4<f32> = textureGather(arg_0, arg_1, vec2<f32>(1.0f), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_2a4f40();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_2a4f40();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_2a4f40();
}
