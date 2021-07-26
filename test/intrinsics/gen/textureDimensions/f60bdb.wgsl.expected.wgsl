[[group(1), binding(0)]] var arg_0 : texture_depth_multisampled_2d;

fn textureDimensions_f60bdb() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_f60bdb();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_f60bdb();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_f60bdb();
}
