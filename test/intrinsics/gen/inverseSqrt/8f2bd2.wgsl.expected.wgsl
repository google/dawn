fn inverseSqrt_8f2bd2() {
  var res : vec2<f32> = inverseSqrt(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  inverseSqrt_8f2bd2();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  inverseSqrt_8f2bd2();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  inverseSqrt_8f2bd2();
}
