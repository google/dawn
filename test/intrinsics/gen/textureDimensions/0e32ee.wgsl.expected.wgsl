[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<rgba16uint>;

fn textureDimensions_0e32ee() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_0e32ee();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_0e32ee();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_0e32ee();
}
