fn length_056071() {
  var res : f32 = length(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  length_056071();
}

[[stage(fragment)]]
fn fragment_main() {
  length_056071();
}

[[stage(compute)]]
fn compute_main() {
  length_056071();
}
