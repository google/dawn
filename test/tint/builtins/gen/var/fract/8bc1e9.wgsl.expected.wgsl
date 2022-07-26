fn fract_8bc1e9() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = fract(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_8bc1e9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_8bc1e9();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_8bc1e9();
}
