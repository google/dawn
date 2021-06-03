[[group(1), binding(0)]] var arg_0 : texture_3d<i32>;

fn textureDimensions_efc8a4() {
  var res : vec3<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_efc8a4();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_efc8a4();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_efc8a4();
}
