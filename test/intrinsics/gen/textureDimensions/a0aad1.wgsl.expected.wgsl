[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d<r32sint>;

fn textureDimensions_a0aad1() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_a0aad1();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_a0aad1();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_a0aad1();
}
