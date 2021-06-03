[[group(1), binding(0)]] var arg_0 : texture_depth_2d;

fn textureLoad_19cf87() {
  var res : f32 = textureLoad(arg_0, vec2<i32>(), 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_19cf87();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_19cf87();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_19cf87();
}
