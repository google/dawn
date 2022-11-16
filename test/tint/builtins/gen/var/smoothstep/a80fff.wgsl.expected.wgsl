fn smoothstep_a80fff() {
  const arg_0 = 2.0;
  const arg_1 = 4.0;
  const arg_2 = 3.0;
  var res = smoothstep(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_a80fff();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_a80fff();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_a80fff();
}
