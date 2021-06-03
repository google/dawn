[[group(1), binding(0)]] var arg_0 : texture_2d<f32>;

fn textureDimensions_2fe1cc() {
  var res : vec2<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_2fe1cc();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_2fe1cc();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_2fe1cc();
}
