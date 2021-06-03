fn dpdxCoarse_f64d7b() {
  var res : vec3<f32> = dpdxCoarse(vec3<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  dpdxCoarse_f64d7b();
}
