[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<rgba32uint>;

fn textureDimensions_68105c() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_68105c();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_68105c();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_68105c();
}
