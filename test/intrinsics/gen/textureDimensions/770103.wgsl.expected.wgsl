[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<r32float>;

fn textureDimensions_770103() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_770103();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_770103();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_770103();
}
