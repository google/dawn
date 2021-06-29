fn any_e755c1() {
  var res : bool = any(vec3<bool>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  any_e755c1();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  any_e755c1();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  any_e755c1();
}
