fn isFinite_34d32b() {
  var res : vec2<bool> = isFinite(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  isFinite_34d32b();
}

[[stage(fragment)]]
fn fragment_main() {
  isFinite_34d32b();
}

[[stage(compute)]]
fn compute_main() {
  isFinite_34d32b();
}
