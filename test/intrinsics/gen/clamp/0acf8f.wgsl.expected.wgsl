fn clamp_0acf8f() {
  var res : vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  clamp_0acf8f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_0acf8f();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  clamp_0acf8f();
}
