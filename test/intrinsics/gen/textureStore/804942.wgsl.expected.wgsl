[[group(1), binding(0)]] var arg_0 : texture_storage_2d<r32sint, write>;

fn textureStore_804942() {
  textureStore(arg_0, vec2<i32>(), vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureStore_804942();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_804942();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureStore_804942();
}
