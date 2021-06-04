[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rg32float>;

fn textureDimensions_214b7b() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_214b7b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_214b7b();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_214b7b();
}
