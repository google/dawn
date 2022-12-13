fn fract_2eddfe() {
  const arg_0 = 1.25;
  var res = fract(arg_0);
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
