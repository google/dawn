fn dpdxCoarse_9581cf() {
  var res : vec2<f32> = dpdxCoarse(vec2<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@fragment
fn fragment_main() {
  dpdxCoarse_9581cf();
}
