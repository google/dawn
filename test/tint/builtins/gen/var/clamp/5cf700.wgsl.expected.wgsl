fn clamp_5cf700() {
  const arg_0 = vec3(1.0);
  const arg_1 = vec3(1.0);
  const arg_2 = vec3(1.0);
  var res = clamp(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_5cf700();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_5cf700();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_5cf700();
}
