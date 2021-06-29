[[group(1), binding(0)]] var arg_0 : texture_storage_3d<rgba32float, write>;

fn textureStore_331aee() {
  textureStore(arg_0, vec3<i32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureStore_331aee();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_331aee();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureStore_331aee();
}
