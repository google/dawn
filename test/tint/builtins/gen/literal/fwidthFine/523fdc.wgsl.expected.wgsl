fn fwidthFine_523fdc() {
  var res : vec3<f32> = fwidthFine(vec3<f32>(1.0f));
}

@fragment
fn fragment_main() {
  fwidthFine_523fdc();
}
