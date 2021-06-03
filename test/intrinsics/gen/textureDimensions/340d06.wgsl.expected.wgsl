[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<r32uint>;

fn textureDimensions_340d06() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_340d06();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_340d06();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_340d06();
}
