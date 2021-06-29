[[group(1), binding(0)]] var arg_0 : texture_storage_3d<rgba16float, read>;

fn textureDimensions_56ccfa() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_56ccfa();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_56ccfa();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_56ccfa();
}
