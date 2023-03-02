fn round_773a8f() {
  var res = round(3.5);
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
