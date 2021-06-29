fn max_0c0aae() {
  var res : u32 = max(1u, 1u);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  max_0c0aae();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  max_0c0aae();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  max_0c0aae();
}
