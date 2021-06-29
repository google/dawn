[[group(1), binding(0)]] var arg_0 : texture_storage_3d<r32uint, write>;

fn textureStore_ef9f2f() {
  textureStore(arg_0, vec3<i32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureStore_ef9f2f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_ef9f2f();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureStore_ef9f2f();
}
