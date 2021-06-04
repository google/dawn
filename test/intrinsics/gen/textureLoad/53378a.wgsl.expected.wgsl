[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d<rg32sint>;

fn textureLoad_53378a() {
  var res : vec4<i32> = textureLoad(arg_0, vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_53378a();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_53378a();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_53378a();
}
