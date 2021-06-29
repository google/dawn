[[group(1), binding(0)]] var arg_0 : texture_storage_2d<rgba8uint, write>;

fn textureStore_77c0ae() {
  textureStore(arg_0, vec2<i32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureStore_77c0ae();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_77c0ae();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureStore_77c0ae();
}
