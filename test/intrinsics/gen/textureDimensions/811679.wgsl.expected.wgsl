[[group(1), binding(0)]] var arg_0 : texture_storage_3d<rgba32uint, write>;

fn textureDimensions_811679() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_811679();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_811679();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_811679();
}
