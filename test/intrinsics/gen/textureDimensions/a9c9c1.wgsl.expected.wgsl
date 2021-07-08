[[group(1), binding(0)]] var arg_0 : texture_cube<f32>;

fn textureDimensions_a9c9c1() {
  var res : vec2<i32> = textureDimensions(arg_0, 0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_a9c9c1();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_a9c9c1();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_a9c9c1();
}
