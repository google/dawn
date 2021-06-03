fn frexp_4b73e3() {
  var arg_1 : vec4<i32>;
  var res : vec4<f32> = frexp(vec4<f32>(), &(arg_1));
}

[[stage(vertex)]]
fn vertex_main() {
  frexp_4b73e3();
}

[[stage(fragment)]]
fn fragment_main() {
  frexp_4b73e3();
}

[[stage(compute)]]
fn compute_main() {
  frexp_4b73e3();
}
