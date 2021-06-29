fn max_453e04() {
  var res : vec4<u32> = max(vec4<u32>(), vec4<u32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  max_453e04();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  max_453e04();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  max_453e04();
}
