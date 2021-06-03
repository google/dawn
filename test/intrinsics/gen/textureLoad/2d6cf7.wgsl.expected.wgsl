[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rg32sint>;

fn textureLoad_2d6cf7() {
  var res : vec4<i32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_2d6cf7();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_2d6cf7();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_2d6cf7();
}
