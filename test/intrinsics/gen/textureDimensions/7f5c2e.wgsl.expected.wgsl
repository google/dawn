[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<rg32sint>;

fn textureDimensions_7f5c2e() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() {
  textureDimensions_7f5c2e();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_7f5c2e();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_7f5c2e();
}
