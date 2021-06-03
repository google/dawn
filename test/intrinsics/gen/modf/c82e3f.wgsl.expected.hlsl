SKIP: FAILED


fn modf_c82e3f() {
  var arg_1 : vec4<f32>;
  var res : vec4<f32> = modf(vec4<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() {
  modf_c82e3f();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_c82e3f();
}

[[stage(compute)]]
fn compute_main() {
  modf_c82e3f();
}

Failed to generate: error: Unknown builtin method: modf
