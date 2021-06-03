[[group(1), binding(0)]] var arg_0 : texture_cube_array<u32>;

fn textureDimensions_183b3e() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_183b3e();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_183b3e();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_183b3e();
}
