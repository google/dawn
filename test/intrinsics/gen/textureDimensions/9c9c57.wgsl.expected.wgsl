[[group(1), binding(0)]] var arg_0 : texture_2d_array<i32>;

fn textureDimensions_9c9c57() {
  var res : vec2<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_9c9c57();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_9c9c57();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_9c9c57();
}
