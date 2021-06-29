[[group(1), binding(0)]] var arg_0 : texture_storage_3d<rgba8uint, write>;

fn textureStore_f8dead() {
  textureStore(arg_0, vec3<i32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureStore_f8dead();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_f8dead();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureStore_f8dead();
}
