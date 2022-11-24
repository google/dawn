fn inverseSqrt_f60c1c() {
  var res = inverseSqrt(vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  inverseSqrt_f60c1c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  inverseSqrt_f60c1c();
}

@compute @workgroup_size(1)
fn compute_main() {
  inverseSqrt_f60c1c();
}
