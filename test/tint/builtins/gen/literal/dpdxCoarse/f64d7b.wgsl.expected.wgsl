fn dpdxCoarse_f64d7b() {
  var res : vec3<f32> = dpdxCoarse(vec3<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@fragment
fn fragment_main() {
  dpdxCoarse_f64d7b();
}
