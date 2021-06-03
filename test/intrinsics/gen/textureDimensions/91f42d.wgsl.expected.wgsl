[[group(1), binding(0)]] var arg_0 : texture_cube_array<u32>;

fn textureDimensions_91f42d() {
  var res : vec3<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_91f42d();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_91f42d();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_91f42d();
}
