[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<rgba32sint>;

fn textureDimensions_bb3dde() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_bb3dde();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_bb3dde();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_bb3dde();
}
