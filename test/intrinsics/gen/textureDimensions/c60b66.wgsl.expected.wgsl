[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rgba16sint>;

fn textureDimensions_c60b66() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_c60b66();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_c60b66();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_c60b66();
}
