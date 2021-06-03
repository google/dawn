[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d<rgba32uint>;

fn textureDimensions_c74533() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_c74533();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_c74533();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_c74533();
}
