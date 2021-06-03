fn exp_0f70eb() {
  var res : vec4<f32> = exp(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  exp_0f70eb();
}

[[stage(fragment)]]
fn fragment_main() {
  exp_0f70eb();
}

[[stage(compute)]]
fn compute_main() {
  exp_0f70eb();
}
