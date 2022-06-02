fn ldexp_a31cdc() {
  var arg_0 = vec3<f32>();
  var arg_1 = vec3<i32>();
  var res : vec3<f32> = ldexp(arg_0, arg_1);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_a31cdc();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  ldexp_a31cdc();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  ldexp_a31cdc();
}
