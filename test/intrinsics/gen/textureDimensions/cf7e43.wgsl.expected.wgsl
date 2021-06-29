[[group(1), binding(0)]] var arg_0 : texture_storage_3d<rgba8snorm, write>;

fn textureDimensions_cf7e43() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_cf7e43();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_cf7e43();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_cf7e43();
}
