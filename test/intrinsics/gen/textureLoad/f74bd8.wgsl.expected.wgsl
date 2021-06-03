[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<rg32float>;

fn textureLoad_f74bd8() {
  var res : vec4<f32> = textureLoad(arg_0, vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_f74bd8();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_f74bd8();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_f74bd8();
}
