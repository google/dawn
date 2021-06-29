fn select_ebfea2() {
  var res : vec3<f32> = select(vec3<f32>(), vec3<f32>(), vec3<bool>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_ebfea2();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_ebfea2();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_ebfea2();
}
