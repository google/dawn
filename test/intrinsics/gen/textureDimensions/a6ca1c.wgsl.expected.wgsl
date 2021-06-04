[[group(1), binding(0)]] var arg_0 : texture_depth_cube;

fn textureDimensions_a6ca1c() {
  var res : vec3<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_a6ca1c();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_a6ca1c();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_a6ca1c();
}
