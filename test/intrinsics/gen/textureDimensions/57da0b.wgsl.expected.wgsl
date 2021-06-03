[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_1d<r32uint>;

fn textureDimensions_57da0b() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_57da0b();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_57da0b();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_57da0b();
}
