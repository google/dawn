fn distance_cfed73() {
  var arg_0 = 1.0;
  var arg_1 = 1.0;
  var res : f32 = distance(arg_0, arg_1);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_cfed73();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  distance_cfed73();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  distance_cfed73();
}
