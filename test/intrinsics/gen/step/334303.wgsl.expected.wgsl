fn step_334303() {
  var res : vec3<f32> = step(vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  step_334303();
}

[[stage(fragment)]]
fn fragment_main() {
  step_334303();
}

[[stage(compute)]]
fn compute_main() {
  step_334303();
}
