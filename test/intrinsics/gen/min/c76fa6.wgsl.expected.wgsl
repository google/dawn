fn min_c76fa6() {
  var res : vec4<f32> = min(vec4<f32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  min_c76fa6();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  min_c76fa6();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  min_c76fa6();
}
