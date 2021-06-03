[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rg32uint>;

fn textureDimensions_441392() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_441392();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_441392();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_441392();
}
