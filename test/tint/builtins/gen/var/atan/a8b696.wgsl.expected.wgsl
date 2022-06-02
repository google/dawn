fn atan_a8b696() {
  var arg_0 = vec4<f32>();
  var res : vec4<f32> = atan(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_a8b696();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  atan_a8b696();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  atan_a8b696();
}
