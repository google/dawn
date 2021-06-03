[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_3d<rg32sint>;

fn textureDimensions_60bf54() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_60bf54();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_60bf54();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_60bf54();
}
