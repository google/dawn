fn dot_883f0e() {
  var res : f32 = dot(vec2<f32>(), vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  dot_883f0e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  dot_883f0e();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  dot_883f0e();
}
