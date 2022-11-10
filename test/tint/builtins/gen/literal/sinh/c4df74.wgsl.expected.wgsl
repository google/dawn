fn sinh_c4df74() {
  var res = sinh(1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sinh_c4df74();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sinh_c4df74();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_c4df74();
}
