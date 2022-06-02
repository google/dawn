fn trunc_eb83df() {
  var arg_0 = 1.0;
  var res : f32 = trunc(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  trunc_eb83df();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  trunc_eb83df();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  trunc_eb83df();
}
