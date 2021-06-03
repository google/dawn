[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<rg32sint>;

fn textureDimensions_e93464() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_e93464();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_e93464();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_e93464();
}
