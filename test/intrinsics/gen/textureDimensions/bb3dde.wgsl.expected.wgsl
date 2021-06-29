[[group(1), binding(0)]] var arg_0 : texture_storage_3d<rgba32sint, write>;

fn textureDimensions_bb3dde() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_bb3dde();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_bb3dde();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_bb3dde();
}
