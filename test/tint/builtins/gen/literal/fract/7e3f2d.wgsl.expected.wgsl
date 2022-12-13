fn fract_7e3f2d() {
  var res = fract(vec4(1.25));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_7e3f2d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_7e3f2d();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_7e3f2d();
}
