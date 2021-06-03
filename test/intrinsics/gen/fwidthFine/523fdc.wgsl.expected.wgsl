fn fwidthFine_523fdc() {
  var res : vec3<f32> = fwidthFine(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  fwidthFine_523fdc();
}

[[stage(fragment)]]
fn fragment_main() {
  fwidthFine_523fdc();
}

[[stage(compute)]]
fn compute_main() {
  fwidthFine_523fdc();
}
