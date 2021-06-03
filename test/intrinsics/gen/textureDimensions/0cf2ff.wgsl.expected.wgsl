[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<rgba16uint>;

fn textureDimensions_0cf2ff() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_0cf2ff();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_0cf2ff();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_0cf2ff();
}
