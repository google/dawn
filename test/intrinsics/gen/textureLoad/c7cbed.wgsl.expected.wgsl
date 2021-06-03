[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<r32float>;

fn textureLoad_c7cbed() {
  var res : vec4<f32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_c7cbed();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_c7cbed();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_c7cbed();
}
