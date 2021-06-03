[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<rgba32sint>;

fn textureDimensions_83ee5a() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_83ee5a();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_83ee5a();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_83ee5a();
}
