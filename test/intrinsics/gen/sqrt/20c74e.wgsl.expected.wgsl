fn sqrt_20c74e() {
  var res : f32 = sqrt(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  sqrt_20c74e();
}

[[stage(fragment)]]
fn fragment_main() {
  sqrt_20c74e();
}

[[stage(compute)]]
fn compute_main() {
  sqrt_20c74e();
}
