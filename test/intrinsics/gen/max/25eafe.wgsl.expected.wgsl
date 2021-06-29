fn max_25eafe() {
  var res : vec3<i32> = max(vec3<i32>(), vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  max_25eafe();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  max_25eafe();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  max_25eafe();
}
