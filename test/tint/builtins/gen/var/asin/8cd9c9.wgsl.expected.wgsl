fn asin_8cd9c9() {
  var arg_0 = vec3<f32>(0.47942554950714111328f);
  var res : vec3<f32> = asin(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_8cd9c9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_8cd9c9();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_8cd9c9();
}
