fn sinh_c9a5eb() {
  var res : vec3<f32> = sinh(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  sinh_c9a5eb();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  sinh_c9a5eb();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  sinh_c9a5eb();
}
