[[group(1), binding(0)]] var arg_0 : texture_1d<i32>;

fn textureDimensions_423f99() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_423f99();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_423f99();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_423f99();
}
