fn fract_fa5c71() {
  var arg_0 = 1.0f;
  var res : f32 = fract(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_fa5c71();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_fa5c71();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_fa5c71();
}
