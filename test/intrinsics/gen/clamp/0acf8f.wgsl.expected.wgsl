fn clamp_0acf8f() {
  var res : vec2<f32> = clamp(vec2<f32>(), vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  clamp_0acf8f();
}

[[stage(fragment)]]
fn fragment_main() {
  clamp_0acf8f();
}

[[stage(compute)]]
fn compute_main() {
  clamp_0acf8f();
}
