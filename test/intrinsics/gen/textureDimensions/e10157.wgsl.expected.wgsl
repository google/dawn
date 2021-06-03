[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d<rgba8uint>;

fn textureDimensions_e10157() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_e10157();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_e10157();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_e10157();
}
