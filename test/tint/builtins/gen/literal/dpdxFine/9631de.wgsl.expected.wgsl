fn dpdxFine_9631de() {
  var res : vec2<f32> = dpdxFine(vec2<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@fragment
fn fragment_main() {
  dpdxFine_9631de();
}
