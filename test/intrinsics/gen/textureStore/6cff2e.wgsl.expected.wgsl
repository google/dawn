[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<r32uint>;

fn textureStore_6cff2e() {
  textureStore(arg_0, vec2<i32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureStore_6cff2e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureStore_6cff2e();
}

[[stage(compute)]]
fn compute_main() {
  textureStore_6cff2e();
}
