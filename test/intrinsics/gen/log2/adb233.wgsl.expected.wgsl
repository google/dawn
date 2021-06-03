fn log2_adb233() {
  var res : vec3<f32> = log2(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  log2_adb233();
}

[[stage(fragment)]]
fn fragment_main() {
  log2_adb233();
}

[[stage(compute)]]
fn compute_main() {
  log2_adb233();
}
