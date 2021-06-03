[[group(1), binding(0)]] var arg_0 : texture_2d<f32>;

fn textureDimensions_9f8e46() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_9f8e46();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_9f8e46();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_9f8e46();
}
