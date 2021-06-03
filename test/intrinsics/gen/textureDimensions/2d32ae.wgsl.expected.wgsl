[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<rgba32uint>;

fn textureDimensions_2d32ae() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_2d32ae();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_2d32ae();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_2d32ae();
}
