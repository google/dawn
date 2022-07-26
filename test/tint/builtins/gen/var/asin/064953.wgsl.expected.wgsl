fn asin_064953() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = asin(arg_0);
}

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
