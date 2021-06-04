[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_1d<rgba32sint>;

fn textureDimensions_6adac6() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_6adac6();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_6adac6();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_6adac6();
}
