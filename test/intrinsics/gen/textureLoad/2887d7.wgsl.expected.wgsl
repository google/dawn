[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rgba32float>;

fn textureLoad_2887d7() {
  var res : vec4<f32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_2887d7();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_2887d7();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_2887d7();
}
