[[group(1), binding(0)]] var arg_0 : texture_storage_3d<rgba32uint, write>;

fn textureStore_441ba8() {
  textureStore(arg_0, vec3<i32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureStore_441ba8();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_441ba8();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureStore_441ba8();
}
