SKIP: FAILED


fn frexp_79ed81() {
  var arg_1 : vec3<i32>;
  var res : vec3<f32> = frexp(vec3<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() {
  frexp_79ed81();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_79ed81();
}

[[stage(compute)]]
fn compute_main() {
  frexp_79ed81();
}

Failed to generate: error: Unknown builtin method: frexp
