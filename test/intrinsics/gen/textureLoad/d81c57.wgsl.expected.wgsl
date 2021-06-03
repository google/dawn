[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rg32float>;

fn textureLoad_d81c57() {
  var res : vec4<f32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_d81c57();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_d81c57();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_d81c57();
}
