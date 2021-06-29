fn log2_adb233() {
  var res : vec3<f32> = log2(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  log2_adb233();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  log2_adb233();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  log2_adb233();
}
