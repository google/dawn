fn abs_005174() {
  var res : vec3<f32> = abs(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  abs_005174();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  abs_005174();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  abs_005174();
}
