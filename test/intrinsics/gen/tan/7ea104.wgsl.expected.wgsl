fn tan_7ea104() {
  var res : vec3<f32> = tan(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  tan_7ea104();
}

[[stage(fragment)]]
fn fragment_main() {
  tan_7ea104();
}

[[stage(compute)]]
fn compute_main() {
  tan_7ea104();
}
