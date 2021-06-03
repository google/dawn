[[group(1), binding(0)]] var arg_0 : texture_multisampled_2d<i32>;

fn textureLoad_e3d2cc() {
  var res : vec4<i32> = textureLoad(arg_0, vec2<i32>(), 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_e3d2cc();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_e3d2cc();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_e3d2cc();
}
