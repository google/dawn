fn ldexp_a6126e() {
  var res = ldexp(vec3(1.0), vec3<i32>(1i));
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
