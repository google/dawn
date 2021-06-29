[[group(1), binding(0)]] var arg_0 : texture_storage_1d<rgba8sint, read>;

fn textureDimensions_6e2d12() {
  var res : i32 = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_6e2d12();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_6e2d12();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_6e2d12();
}
