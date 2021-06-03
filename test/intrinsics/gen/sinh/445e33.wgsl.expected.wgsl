fn sinh_445e33() {
  var res : vec4<f32> = sinh(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  sinh_445e33();
}

[[stage(fragment)]]
fn fragment_main() {
  sinh_445e33();
}

[[stage(compute)]]
fn compute_main() {
  sinh_445e33();
}
