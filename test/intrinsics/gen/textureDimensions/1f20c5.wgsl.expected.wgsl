[[group(1), binding(0)]] var arg_0 : texture_2d_array<u32>;

fn textureDimensions_1f20c5() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_1f20c5();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_1f20c5();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_1f20c5();
}
