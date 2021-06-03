fn ceil_678655() {
  var res : f32 = ceil(1.0);
}

[[stage(vertex)]]
fn vertex_main() {
  ceil_678655();
}

[[stage(fragment)]]
fn fragment_main() {
  ceil_678655();
}

[[stage(compute)]]
fn compute_main() {
  ceil_678655();
}
