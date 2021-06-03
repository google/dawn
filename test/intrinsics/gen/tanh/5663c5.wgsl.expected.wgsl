fn tanh_5663c5() {
  var res : vec4<f32> = tanh(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  tanh_5663c5();
}

[[stage(fragment)]]
fn fragment_main() {
  tanh_5663c5();
}

[[stage(compute)]]
fn compute_main() {
  tanh_5663c5();
}
