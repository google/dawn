[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<r32uint>;

fn textureStore_38e8d7() {
  textureStore(arg_0, vec2<i32>(), 1, vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureStore_38e8d7();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_38e8d7();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_38e8d7();
}
