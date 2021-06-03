[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rgba32sint>;

fn textureLoad_ddeed3() {
  var res : vec4<i32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_ddeed3();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_ddeed3();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_ddeed3();
}
