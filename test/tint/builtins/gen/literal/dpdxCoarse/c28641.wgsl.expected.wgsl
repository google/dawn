fn dpdxCoarse_c28641() {
  var res : vec4<f32> = dpdxCoarse(vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  dpdxCoarse_c28641();
}
