fn asin_064953() {
  var res : vec4<f32> = asin(vec4<f32>(0.47942554950714111328f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_064953();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_064953();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_064953();
}
