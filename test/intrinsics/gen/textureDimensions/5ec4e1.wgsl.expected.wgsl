[[group(1), binding(0)]] var arg_0 : texture_cube<u32>;

fn textureDimensions_5ec4e1() {
  var res : vec3<i32> = textureDimensions(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_5ec4e1();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_5ec4e1();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_5ec4e1();
}
