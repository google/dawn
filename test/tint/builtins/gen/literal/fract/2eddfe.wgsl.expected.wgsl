fn fract_2eddfe() {
  var res = fract(1.25);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_2eddfe();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_2eddfe();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_2eddfe();
}
