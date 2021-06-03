[[group(1), binding(0)]] var arg_0 : texture_cube_array<f32>;

fn textureDimensions_3e0403() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_3e0403();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_3e0403();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_3e0403();
}
