fn isFinite_8a23ad() {
  var res : vec3<bool> = isFinite(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  isFinite_8a23ad();
}

[[stage(fragment)]]
fn fragment_main() {
  isFinite_8a23ad();
}

[[stage(compute)]]
fn compute_main() {
  isFinite_8a23ad();
}
