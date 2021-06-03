[[group(1), binding(0)]] var arg_0 : texture_cube_array<f32>;

fn textureDimensions_eda4e3() {
  var res : vec3<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_eda4e3();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_eda4e3();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_eda4e3();
}
