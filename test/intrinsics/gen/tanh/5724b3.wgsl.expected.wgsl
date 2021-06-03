fn tanh_5724b3() {
  var res : vec2<f32> = tanh(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  tanh_5724b3();
}

[[stage(fragment)]]
fn fragment_main() {
  tanh_5724b3();
}

[[stage(compute)]]
fn compute_main() {
  tanh_5724b3();
}
