fn dpdyCoarse_ae1873() -> vec3<f32> {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = dpdyCoarse(arg_0);
  return res;
}

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@fragment
fn fragment_main() {
  prevent_dce = dpdyCoarse_ae1873();
}
