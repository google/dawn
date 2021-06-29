[[group(1), binding(0)]] var arg_0 : texture_storage_3d<r32sint, read>;

fn textureDimensions_1e189c() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_1e189c();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_1e189c();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_1e189c();
}
