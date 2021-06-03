[[group(1), binding(0)]] var arg_0 : texture_2d<f32>;

fn textureLoad_484344() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<i32>(), 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_484344();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_484344();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_484344();
}
