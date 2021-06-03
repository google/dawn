fn exp_1951e7() {
  var res : vec2<f32> = exp(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  exp_1951e7();
}

[[stage(fragment)]]
fn fragment_main() {
  exp_1951e7();
}

[[stage(compute)]]
fn compute_main() {
  exp_1951e7();
}
