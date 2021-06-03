[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<r32sint>;

fn textureLoad_f06b69() {
  var res : vec4<i32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_f06b69();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_f06b69();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_f06b69();
}
