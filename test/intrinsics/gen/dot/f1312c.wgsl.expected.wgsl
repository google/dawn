fn dot_f1312c() {
  var res : i32 = dot(vec3<i32>(), vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  dot_f1312c();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  dot_f1312c();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  dot_f1312c();
}
