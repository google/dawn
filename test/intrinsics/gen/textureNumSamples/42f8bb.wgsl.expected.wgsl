[[group(1), binding(0)]] var arg_0 : texture_multisampled_2d<u32>;

fn textureNumSamples_42f8bb() {
  var res : i32 = textureNumSamples(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumSamples_42f8bb();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumSamples_42f8bb();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumSamples_42f8bb();
}
