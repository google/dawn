[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<rgba16sint>;

fn textureStore_8f71a1() {
  textureStore(arg_0, vec3<i32>(), vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureStore_8f71a1();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_8f71a1();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_8f71a1();
}
