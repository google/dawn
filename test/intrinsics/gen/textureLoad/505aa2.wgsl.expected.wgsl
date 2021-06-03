[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<r32sint>;

fn textureLoad_505aa2() {
  var res : vec4<i32> = textureLoad(arg_0, vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_505aa2();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_505aa2();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_505aa2();
}
