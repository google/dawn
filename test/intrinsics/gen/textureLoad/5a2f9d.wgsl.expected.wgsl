[[group(1), binding(0)]] var arg_0 : texture_1d<i32>;

fn textureLoad_5a2f9d() {
  var res : vec4<i32> = textureLoad(arg_0, 1, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_5a2f9d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_5a2f9d();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_5a2f9d();
}
