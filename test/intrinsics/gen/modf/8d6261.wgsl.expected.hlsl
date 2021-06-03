SKIP: FAILED


fn modf_8d6261() {
  var arg_1 : f32;
  var res : f32 = modf(1.0, &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() {
  modf_8d6261();
}

[[stage(fragment)]]
fn fragment_main() {
  modf_8d6261();
}

[[stage(compute)]]
fn compute_main() {
  modf_8d6261();
}

Failed to generate: error: Unknown builtin method: modf
