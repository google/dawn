fn fma_eb25d7() {
  const arg_0 = vec3(1.0);
  const arg_1 = vec3(1.0);
  const arg_2 = vec3(1.0);
  var res = fma(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_eb25d7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_eb25d7();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_eb25d7();
}
