fn smoothstep_a80fff() {
  var res = smoothstep(2.0, 4.0, 3.0);
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
