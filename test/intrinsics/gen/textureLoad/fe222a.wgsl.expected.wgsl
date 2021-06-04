[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rgba8snorm>;

fn textureLoad_fe222a() {
  var res : vec4<f32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_fe222a();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_fe222a();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_fe222a();
}
