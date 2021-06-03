fn trunc_562d05() {
  var res : vec3<f32> = trunc(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() {
  trunc_562d05();
}

[[stage(fragment)]]
fn fragment_main() {
  trunc_562d05();
}

[[stage(compute)]]
fn compute_main() {
  trunc_562d05();
}
