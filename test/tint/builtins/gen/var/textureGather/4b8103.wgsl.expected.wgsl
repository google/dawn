@group(1) @binding(1) var arg_1 : texture_2d_array<f32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_4b8103() {
  const arg_0 = 1;
  var arg_3 = vec2<f32>();
  var arg_4 = 1;
  const arg_5 = vec2<i32>();
  var res : vec4<f32> = textureGather(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_4b8103();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_4b8103();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_4b8103();
}
