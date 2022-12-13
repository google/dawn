fn fract_ed2f79() {
  var res = fract(vec3(1.25));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_ed2f79();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_ed2f79();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_ed2f79();
}
