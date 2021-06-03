[[group(1), binding(0)]] var arg_0 : texture_depth_2d;

fn textureDimensions_939fdb() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_939fdb();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_939fdb();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_939fdb();
}
