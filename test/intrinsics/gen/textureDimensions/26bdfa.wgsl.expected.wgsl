[[group(1), binding(0)]] var arg_0 : texture_3d<f32>;

fn textureDimensions_26bdfa() {
  var res : vec3<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_26bdfa();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_26bdfa();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_26bdfa();
}
