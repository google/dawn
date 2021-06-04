[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_1d<rg32float>;

fn textureDimensions_55b23e() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_55b23e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_55b23e();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_55b23e();
}
