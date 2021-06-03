[[group(1), binding(0)]] var arg_0 : texture_2d<u32>;

fn textureDimensions_f7145b() {
  var res : vec2<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_f7145b();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_f7145b();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_f7145b();
}
