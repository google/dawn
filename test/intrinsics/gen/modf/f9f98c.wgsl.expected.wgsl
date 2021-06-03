fn modf_f9f98c() {
  var arg_1 : vec3<f32>;
  var res : vec3<f32> = modf(vec3<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() {
  modf_f9f98c();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_f9f98c();
}

[[stage(compute)]]
fn compute_main() {
  modf_f9f98c();
}
