fn exp2_a9d0a7() {
  var res : vec4<f32> = exp2(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  exp2_a9d0a7();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  exp2_a9d0a7();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  exp2_a9d0a7();
}
