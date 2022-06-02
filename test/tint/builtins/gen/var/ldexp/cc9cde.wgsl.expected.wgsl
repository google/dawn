fn ldexp_cc9cde() {
  var arg_0 = vec4<f32>();
  var arg_1 = vec4<i32>();
  var res : vec4<f32> = ldexp(arg_0, arg_1);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_cc9cde();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  ldexp_cc9cde();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  ldexp_cc9cde();
}
