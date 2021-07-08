[[group(1), binding(0)]] var arg_0 : texture_depth_2d_array;

fn textureLoad_9b2667() {
  var res : f32 = textureLoad(arg_0, vec2<i32>(), 1, 0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_9b2667();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_9b2667();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_9b2667();
}
