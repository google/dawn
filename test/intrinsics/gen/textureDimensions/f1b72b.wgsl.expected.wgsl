[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d<rgba32float>;

fn textureDimensions_f1b72b() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_f1b72b();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_f1b72b();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_f1b72b();
}
