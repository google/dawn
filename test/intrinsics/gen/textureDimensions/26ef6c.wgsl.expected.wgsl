[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<rgba8uint>;

fn textureDimensions_26ef6c() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_26ef6c();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_26ef6c();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_26ef6c();
}
