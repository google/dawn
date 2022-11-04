fn atan_7a2a75() {
  const arg_0 = 1.0;
  var res = atan(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_7a2a75();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_7a2a75();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_7a2a75();
}
