[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<rgba8unorm>;

fn textureDimensions_cd76a7() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_cd76a7();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_cd76a7();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_cd76a7();
}
