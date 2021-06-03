[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_1d<r32sint>;

fn textureDimensions_0cce40() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_0cce40();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_0cce40();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_0cce40();
}
