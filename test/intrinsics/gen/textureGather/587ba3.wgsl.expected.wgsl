[[group(1), binding(1)]] var arg_1 : texture_2d<i32>;

[[group(1), binding(2)]] var arg_2 : sampler;

fn textureGather_587ba3() {
  var res : vec4<i32> = textureGather(1, arg_1, arg_2, vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureGather_587ba3();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureGather_587ba3();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureGather_587ba3();
}
