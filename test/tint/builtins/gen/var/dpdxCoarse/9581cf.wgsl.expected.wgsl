fn dpdxCoarse_9581cf() {
  var arg_0 = vec2<f32>();
  var res : vec2<f32> = dpdxCoarse(arg_0);
}

@stage(fragment)
fn fragment_main() {
  dpdxCoarse_9581cf();
}
