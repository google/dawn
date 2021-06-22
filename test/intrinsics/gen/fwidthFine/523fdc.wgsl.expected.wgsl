fn fwidthFine_523fdc() {
  var res : vec3<f32> = fwidthFine(vec3<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  fwidthFine_523fdc();
}
