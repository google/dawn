[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rgba32sint>;

fn textureDimensions_08a62e() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_08a62e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_08a62e();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_08a62e();
}
