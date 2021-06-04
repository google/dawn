[[group(1), binding(0)]] var arg_0 : texture_cube_array<i32>;

fn textureDimensions_f70326() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_f70326();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_f70326();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_f70326();
}
