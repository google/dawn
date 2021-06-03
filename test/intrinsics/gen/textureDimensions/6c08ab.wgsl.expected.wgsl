[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rgba32float>;

fn textureDimensions_6c08ab() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_6c08ab();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_6c08ab();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_6c08ab();
}
