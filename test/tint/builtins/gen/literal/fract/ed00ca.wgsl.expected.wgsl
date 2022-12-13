fn fract_ed00ca() {
  var res = fract(vec2(1.25));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_ed00ca();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_ed00ca();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_ed00ca();
}
