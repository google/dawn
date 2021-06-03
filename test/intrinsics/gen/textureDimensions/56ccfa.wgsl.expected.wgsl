[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<rgba16float>;

fn textureDimensions_56ccfa() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_56ccfa();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_56ccfa();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_56ccfa();
}
