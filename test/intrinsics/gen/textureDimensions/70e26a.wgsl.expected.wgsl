[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_1d<r32uint>;

fn textureDimensions_70e26a() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_70e26a();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_70e26a();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_70e26a();
}
