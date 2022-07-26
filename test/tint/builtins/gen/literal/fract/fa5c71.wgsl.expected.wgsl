fn fract_fa5c71() {
  var res : f32 = fract(1.0f);
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
