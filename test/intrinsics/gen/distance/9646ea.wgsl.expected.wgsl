fn distance_9646ea() {
  var res : f32 = distance(vec4<f32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  distance_9646ea();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  distance_9646ea();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  distance_9646ea();
}
