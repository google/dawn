[[group(1), binding(0)]] var arg_0 : texture_depth_cube_array;

fn textureDimensions_3e626d() {
  var res : vec3<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_3e626d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_3e626d();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_3e626d();
}
