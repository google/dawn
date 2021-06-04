[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<rgba8sint>;

fn textureDimensions_a0e4ec() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_a0e4ec();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_a0e4ec();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_a0e4ec();
}
