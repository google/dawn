[[group(1), binding(0)]] var arg_0 : texture_depth_2d_array;

fn textureDimensions_72e5d6() {
  var res : vec2<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_72e5d6();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_72e5d6();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_72e5d6();
}
