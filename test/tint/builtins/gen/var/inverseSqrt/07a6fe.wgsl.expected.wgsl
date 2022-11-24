fn inverseSqrt_07a6fe() {
  const arg_0 = vec4(1.0);
  var res = inverseSqrt(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  inverseSqrt_07a6fe();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  inverseSqrt_07a6fe();
}

@compute @workgroup_size(1)
fn compute_main() {
  inverseSqrt_07a6fe();
}
