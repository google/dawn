[[group(1), binding(0)]] var arg_0 : texture_storage_2d<rgba16uint, read>;

fn textureDimensions_82138e() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_82138e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_82138e();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_82138e();
}
