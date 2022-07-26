fn fwidthFine_68f4ef() {
  var res : vec4<f32> = fwidthFine(vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  fwidthFine_68f4ef();
}
