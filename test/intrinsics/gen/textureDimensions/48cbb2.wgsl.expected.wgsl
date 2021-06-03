[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d_array<rg32float>;

fn textureDimensions_48cbb2() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_48cbb2();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_48cbb2();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_48cbb2();
}
