fn ceil_34064b() {
  var res : vec3<f32> = ceil(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ceil_34064b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ceil_34064b();
}

[[stage(compute)]]
fn compute_main() {
  ceil_34064b();
}
