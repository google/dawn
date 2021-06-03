[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rgba16uint>;

fn textureDimensions_3d5817() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_3d5817();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_3d5817();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_3d5817();
}
