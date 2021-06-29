[[group(1), binding(0)]] var arg_0 : texture_storage_3d<rg32uint, read>;

fn textureDimensions_924742() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_924742();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_924742();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_924742();
}
