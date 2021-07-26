[[group(1), binding(0)]] var arg_0 : texture_external;

fn textureLoad_8acf41() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_8acf41();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_8acf41();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_8acf41();
}
