fn tan_2f030e() {
  var res : f32 = tan(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  tan_2f030e();
}

[[stage(fragment)]]
fn fragment_main() {
  tan_2f030e();
}

[[stage(compute)]]
fn compute_main() {
  tan_2f030e();
}
