fn dpdxCoarse_9581cf() {
  var res : vec2<f32> = dpdxCoarse(vec2<f32>());
}

[[stage(fragment)]]
fn fragment_main() {
  dpdxCoarse_9581cf();
}
