fn sinh_b9860e() {
  var res : vec2<f32> = sinh(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  sinh_b9860e();
}

[[stage(fragment)]]
fn fragment_main() {
  sinh_b9860e();
}

[[stage(compute)]]
fn compute_main() {
  sinh_b9860e();
}
