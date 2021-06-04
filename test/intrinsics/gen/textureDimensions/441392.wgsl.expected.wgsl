[[group(1), binding(0)]] var arg_0 : texture_storage_1d<rg32uint, read>;

fn textureDimensions_441392() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_441392();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_441392();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_441392();
}
