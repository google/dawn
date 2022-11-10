fn sinh_a3da7c() {
  var res = sinh(vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sinh_a3da7c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sinh_a3da7c();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_a3da7c();
}
