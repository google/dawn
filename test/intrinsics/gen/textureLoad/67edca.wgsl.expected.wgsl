[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<rgba32uint>;

fn textureLoad_67edca() {
  var res : vec4<u32> = textureLoad(arg_0, vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_67edca();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_67edca();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_67edca();
}
