fn min_a45171() {
  var res : vec3<i32> = min(vec3<i32>(), vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  min_a45171();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  min_a45171();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  min_a45171();
}
