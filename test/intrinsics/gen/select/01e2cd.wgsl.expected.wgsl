fn select_01e2cd() {
  var res : vec3<i32> = select(vec3<i32>(), vec3<i32>(), vec3<bool>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  select_01e2cd();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  select_01e2cd();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  select_01e2cd();
}
