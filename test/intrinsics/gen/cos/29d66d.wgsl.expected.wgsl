fn cos_29d66d() {
  var res : vec4<f32> = cos(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  cos_29d66d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  cos_29d66d();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  cos_29d66d();
}
