[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d_array<rgba32sint>;

fn textureLoad_a6b61d() {
  var res : vec4<i32> = textureLoad(arg_0, vec2<i32>(), 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_a6b61d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_a6b61d();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_a6b61d();
}
