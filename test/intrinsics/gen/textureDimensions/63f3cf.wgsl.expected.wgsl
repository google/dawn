[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<rg32float>;

fn textureDimensions_63f3cf() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_63f3cf();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_63f3cf();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_63f3cf();
}
