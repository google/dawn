[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<rgba32float>;

fn textureLoad_3d9c90() {
  var res : vec4<f32> = textureLoad(arg_0, vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_3d9c90();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_3d9c90();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_3d9c90();
}
