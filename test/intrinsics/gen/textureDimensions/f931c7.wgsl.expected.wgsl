[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_2d<r32float>;

fn textureDimensions_f931c7() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_f931c7();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_f931c7();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_f931c7();
}
