[[group(1), binding(0)]] var arg_0 : texture_multisampled_2d<f32>;

fn textureLoad_a583c9() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<i32>(), 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_a583c9();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_a583c9();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_a583c9();
}
