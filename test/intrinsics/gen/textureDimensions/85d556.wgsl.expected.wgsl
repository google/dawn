[[group(1), binding(0)]] var arg_0 : texture_2d_array<f32>;

fn textureDimensions_85d556() {
  var res : vec2<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_85d556();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_85d556();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_85d556();
}
