[[group(1), binding(0)]] var arg_0 : texture_multisampled_2d<i32>;

fn textureNumSamples_449d23() {
  var res : i32 = textureNumSamples(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureNumSamples_449d23();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureNumSamples_449d23();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureNumSamples_449d23();
}
