fn round_773a8f() {
  const arg_0 = 3.5;
  var res = round(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_773a8f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_773a8f();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_773a8f();
}
