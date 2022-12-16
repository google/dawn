fn ldexp_a6126e() {
  const arg_0 = vec3(1.0);
  var arg_1 = vec3<i32>(1i);
  var res = ldexp(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_a6126e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_a6126e();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_a6126e();
}
