[[group(1), binding(0)]] var arg_0 : texture_1d<f32>;

fn textureLoad_81c381() {
  var res : vec4<f32> = textureLoad(arg_0, 1, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_81c381();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_81c381();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_81c381();
}
