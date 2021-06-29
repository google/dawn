fn smoothStep_cb0bfb() {
  var res : f32 = smoothStep(1.0, 1.0, 1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  smoothStep_cb0bfb();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  smoothStep_cb0bfb();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  smoothStep_cb0bfb();
}
