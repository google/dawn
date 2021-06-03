fn cross_041cb0() {
  var res : vec3<f32> = cross(vec3<f32>(), vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  cross_041cb0();
}

[[stage(fragment)]]
fn fragment_main() {
  cross_041cb0();
}

[[stage(compute)]]
fn compute_main() {
  cross_041cb0();
}
