fn sinh_77a2a3() {
  const arg_0 = vec3(1.0);
  var res = sinh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sinh_77a2a3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sinh_77a2a3();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_77a2a3();
}
