fn abs_c3321c() {
  const arg_0 = vec3(1);
  var res = abs(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_c3321c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_c3321c();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_c3321c();
}
