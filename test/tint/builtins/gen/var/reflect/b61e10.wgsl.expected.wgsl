fn reflect_b61e10() {
  var arg_0 = vec2<f32>();
  var arg_1 = vec2<f32>();
  var res : vec2<f32> = reflect(arg_0, arg_1);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  reflect_b61e10();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  reflect_b61e10();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  reflect_b61e10();
}
