fn smoothstep_66e4bd() {
  const arg_0 = vec3(2.0);
  const arg_1 = vec3(4.0);
  const arg_2 = vec3(3.0);
  var res = smoothstep(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_66e4bd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_66e4bd();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_66e4bd();
}
