fn fract_ed2f79() {
  const arg_0 = vec3(1.25);
  var res = fract(arg_0);
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
