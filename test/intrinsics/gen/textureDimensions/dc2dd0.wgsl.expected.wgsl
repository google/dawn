[[group(1), binding(0)]] var arg_0 : [[access(write)]] texture_storage_1d<rgba8uint>;

fn textureDimensions_dc2dd0() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_dc2dd0();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_dc2dd0();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_dc2dd0();
}
