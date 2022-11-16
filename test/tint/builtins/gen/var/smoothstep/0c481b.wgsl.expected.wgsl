fn smoothstep_0c481b() {
  const arg_0 = vec2(2.0);
  const arg_1 = vec2(4.0);
  const arg_2 = vec2(3.0);
  var res = smoothstep(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_0c481b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_0c481b();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_0c481b();
}
