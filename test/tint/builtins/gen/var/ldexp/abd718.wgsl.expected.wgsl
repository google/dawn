fn ldexp_abd718() {
  var arg_0 = vec2<f32>();
  var arg_1 = vec2<i32>();
  var res : vec2<f32> = ldexp(arg_0, arg_1);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_abd718();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  ldexp_abd718();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  ldexp_abd718();
}
