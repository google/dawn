fn exp2_d6777c() {
  var res : vec2<f32> = exp2(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  exp2_d6777c();
}

[[stage(fragment)]]
fn fragment_main() {
  exp2_d6777c();
}

[[stage(compute)]]
fn compute_main() {
  exp2_d6777c();
}
