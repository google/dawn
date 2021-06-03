fn asin_c0c272() {
  var res : f32 = asin(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  asin_c0c272();
}

[[stage(fragment)]]
fn fragment_main() {
  asin_c0c272();
}

[[stage(compute)]]
fn compute_main() {
  asin_c0c272();
}
