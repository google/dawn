fn cosh_e0c1de() {
  var arg_0 = vec4<f32>();
  var res : vec4<f32> = cosh(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  cosh_e0c1de();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  cosh_e0c1de();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  cosh_e0c1de();
}
