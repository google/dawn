[[group(1), binding(0)]] var arg_0 : texture_storage_1d<r32uint, write>;

fn textureDimensions_57da0b() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_57da0b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_57da0b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_57da0b();
}
