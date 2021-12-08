[[group(1), binding(1)]] var arg_1 : texture_2d<u32>;

[[group(1), binding(2)]] var arg_2 : sampler;

fn textureGather_93003d() {
  var res : vec4<u32> = textureGather(1, arg_1, arg_2, vec2<f32>(), vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureGather_93003d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureGather_93003d();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureGather_93003d();
}
