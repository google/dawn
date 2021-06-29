[[group(1), binding(0)]] var arg_0 : texture_2d<u32>;

fn textureDimensions_1191a5() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_1191a5();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_1191a5();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_1191a5();
}
