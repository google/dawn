[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<r32uint>;

fn textureDimensions_fbbe4d() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_fbbe4d();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_fbbe4d();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_fbbe4d();
}
