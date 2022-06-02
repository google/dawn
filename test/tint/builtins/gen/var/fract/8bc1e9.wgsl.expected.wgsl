fn fract_8bc1e9() {
  var arg_0 = vec4<f32>();
  var res : vec4<f32> = fract(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_8bc1e9();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  fract_8bc1e9();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  fract_8bc1e9();
}
