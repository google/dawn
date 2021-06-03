[[group(1), binding(0)]] var arg_0 : texture_2d_array<f32>;

fn textureLoad_87be85() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<i32>(), 1, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_87be85();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_87be85();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_87be85();
}
