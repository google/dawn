fn smoothStep_658be3() {
  var res : vec3<f32> = smoothStep(vec3<f32>(), vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  smoothStep_658be3();
}

[[stage(fragment)]]
fn fragment_main() {
  smoothStep_658be3();
}

[[stage(compute)]]
fn compute_main() {
  smoothStep_658be3();
}
