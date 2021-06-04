[[group(1), binding(0)]] var arg_0 : texture_cube<f32>;

fn textureDimensions_f507c9() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_f507c9();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_f507c9();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_f507c9();
}
