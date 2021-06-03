[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_1d<rgba8snorm>;

fn textureDimensions_cccc8f() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_cccc8f();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_cccc8f();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_cccc8f();
}
