[[group(1), binding(0)]] var arg_0 : texture_cube<u32>;

fn textureDimensions_1ebd6a() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_1ebd6a();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_1ebd6a();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_1ebd6a();
}
