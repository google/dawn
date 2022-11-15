fn max_a1b196() {
  const arg_0 = vec3(1.0);
  const arg_1 = vec3(1.0);
  var res = max(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_a1b196();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_a1b196();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_a1b196();
}
