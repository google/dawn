[[group(1), binding(0)]] var arg_0 : texture_3d<f32>;

fn textureLoad_1f2016() {
  var res : vec4<f32> = textureLoad(arg_0, vec3<i32>(), 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_1f2016();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_1f2016();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_1f2016();
}
