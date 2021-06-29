fn max_85e6bc() {
  var res : vec4<i32> = max(vec4<i32>(), vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  max_85e6bc();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  max_85e6bc();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  max_85e6bc();
}
