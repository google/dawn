fn dpdxCoarse_c28641() {
  var arg_0 = vec4<f32>();
  var res : vec4<f32> = dpdxCoarse(arg_0);
}

@fragment
fn fragment_main() {
  dpdxCoarse_c28641();
}
