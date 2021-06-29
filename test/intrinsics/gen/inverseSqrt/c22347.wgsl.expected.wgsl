fn inverseSqrt_c22347() {
  var res : vec4<f32> = inverseSqrt(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  inverseSqrt_c22347();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  inverseSqrt_c22347();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  inverseSqrt_c22347();
}
