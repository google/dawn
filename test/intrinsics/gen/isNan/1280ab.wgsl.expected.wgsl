fn isNan_1280ab() {
  var res : vec3<bool> = isNan(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  isNan_1280ab();
}

[[stage(fragment)]]
fn fragment_main() {
  isNan_1280ab();
}

[[stage(compute)]]
fn compute_main() {
  isNan_1280ab();
}
