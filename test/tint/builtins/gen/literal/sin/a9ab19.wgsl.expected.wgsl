fn sin_a9ab19() {
  var res = sin(1.57079632679000003037);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_a9ab19();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_a9ab19();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_a9ab19();
}
