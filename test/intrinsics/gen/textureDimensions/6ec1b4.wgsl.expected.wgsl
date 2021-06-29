[[group(1), binding(0)]] var arg_0 : texture_3d<u32>;

fn textureDimensions_6ec1b4() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_6ec1b4();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_6ec1b4();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_6ec1b4();
}
