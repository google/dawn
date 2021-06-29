fn distance_0657d4() {
  var res : f32 = distance(vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  distance_0657d4();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  distance_0657d4();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  distance_0657d4();
}
