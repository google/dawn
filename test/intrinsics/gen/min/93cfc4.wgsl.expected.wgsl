fn min_93cfc4() {
  var res : vec3<f32> = min(vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  min_93cfc4();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  min_93cfc4();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  min_93cfc4();
}
