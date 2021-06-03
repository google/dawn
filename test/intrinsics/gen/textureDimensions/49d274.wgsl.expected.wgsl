[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d_array<rg32sint>;

fn textureDimensions_49d274() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_49d274();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_49d274();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_49d274();
}
