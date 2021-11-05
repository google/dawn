fn dot_ef6b1d() {
  var res : i32 = dot(vec4<i32>(), vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  dot_ef6b1d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  dot_ef6b1d();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  dot_ef6b1d();
}
