fn atan_d17fb2() {
  const arg_0 = vec4(1.0);
  var res = atan(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_d17fb2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_d17fb2();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_d17fb2();
}
