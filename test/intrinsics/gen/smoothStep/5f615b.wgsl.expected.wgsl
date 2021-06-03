fn smoothStep_5f615b() {
  var res : vec4<f32> = smoothStep(vec4<f32>(), vec4<f32>(), vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  smoothStep_5f615b();
}

[[stage(fragment)]]
fn fragment_main() {
  smoothStep_5f615b();
}

[[stage(compute)]]
fn compute_main() {
  smoothStep_5f615b();
}
