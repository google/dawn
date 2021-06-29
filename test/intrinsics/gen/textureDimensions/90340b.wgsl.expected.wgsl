[[group(1), binding(0)]] var arg_0 : texture_depth_cube_array;

fn textureDimensions_90340b() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_90340b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_90340b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_90340b();
}
