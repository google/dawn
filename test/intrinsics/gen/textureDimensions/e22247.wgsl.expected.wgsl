[[group(1), binding(0)]] var arg_0 : texture_cube_array<i32>;

fn textureDimensions_e22247() {
  var res : vec3<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_e22247();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_e22247();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_e22247();
}
