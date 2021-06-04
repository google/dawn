[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<r32sint>;

fn textureDimensions_c30e75() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_c30e75();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_c30e75();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_c30e75();
}
