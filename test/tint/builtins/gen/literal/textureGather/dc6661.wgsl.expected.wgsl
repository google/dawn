@group(1) @binding(1) var arg_1 : texture_2d<i32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_dc6661() {
  var res : vec4<i32> = textureGather(1u, arg_1, arg_2, vec2<f32>(), vec2<i32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_dc6661();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_dc6661();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_dc6661();
}
