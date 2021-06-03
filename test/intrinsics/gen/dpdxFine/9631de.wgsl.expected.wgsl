fn dpdxFine_9631de() {
  var res : vec2<f32> = dpdxFine(vec2<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  dpdxFine_9631de();
}
