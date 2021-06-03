[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d_array<rgba8sint>;

fn textureDimensions_f7aa9e() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_f7aa9e();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_f7aa9e();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_f7aa9e();
}
