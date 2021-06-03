fn exp_d98450() {
  var res : vec3<f32> = exp(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  exp_d98450();
}

[[stage(fragment)]]
fn fragment_main() {
  exp_d98450();
}

[[stage(compute)]]
fn compute_main() {
  exp_d98450();
}
