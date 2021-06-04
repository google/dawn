[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d<rgba16float>;

fn textureDimensions_89a864() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_89a864();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_89a864();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_89a864();
}
