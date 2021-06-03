[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<rg32uint>;

fn textureDimensions_924742() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_924742();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_924742();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_924742();
}
