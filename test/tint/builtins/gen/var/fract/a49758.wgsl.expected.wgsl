fn fract_a49758() {
  var arg_0 = vec3<f32>();
  var res : vec3<f32> = fract(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_a49758();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  fract_a49758();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  fract_a49758();
}
