fn acos_489247() {
  var arg_0 = 1.0;
  var res : f32 = acos(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_489247();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  acos_489247();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  acos_489247();
}
