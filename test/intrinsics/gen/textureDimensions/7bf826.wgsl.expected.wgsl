[[group(1), binding(0)]] var arg_0 : texture_depth_2d_array;

fn textureDimensions_7bf826() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_7bf826();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_7bf826();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_7bf826();
}
