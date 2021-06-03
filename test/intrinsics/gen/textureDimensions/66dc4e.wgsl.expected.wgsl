[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d_array<rgba8snorm>;

fn textureDimensions_66dc4e() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_66dc4e();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_66dc4e();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_66dc4e();
}
