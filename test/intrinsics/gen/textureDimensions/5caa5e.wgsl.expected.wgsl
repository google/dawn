[[group(1), binding(0)]] var arg_0 : texture_storage_1d<rgba32uint, write>;

fn textureDimensions_5caa5e() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_5caa5e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_5caa5e();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_5caa5e();
}
