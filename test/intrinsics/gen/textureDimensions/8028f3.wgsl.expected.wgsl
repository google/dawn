[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<r32float>;

fn textureDimensions_8028f3() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_8028f3();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_8028f3();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_8028f3();
}
