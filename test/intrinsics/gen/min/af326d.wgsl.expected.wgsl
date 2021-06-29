fn min_af326d() {
  var res : f32 = min(1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  min_af326d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  min_af326d();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  min_af326d();
}
