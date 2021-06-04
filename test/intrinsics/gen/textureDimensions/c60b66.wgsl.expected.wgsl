[[group(1), binding(0)]] var arg_0 : texture_storage_1d<rgba16sint, read>;

fn textureDimensions_c60b66() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_c60b66();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_c60b66();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_c60b66();
}
