[[group(1), binding(0)]] var arg_0 : texture_depth_2d;

fn textureDimensions_12c9bb() {
  var res : vec2<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_12c9bb();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_12c9bb();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_12c9bb();
}
