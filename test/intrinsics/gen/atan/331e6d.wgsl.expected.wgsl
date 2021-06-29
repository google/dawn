fn atan_331e6d() {
  var res : vec3<f32> = atan(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  atan_331e6d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  atan_331e6d();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  atan_331e6d();
}
