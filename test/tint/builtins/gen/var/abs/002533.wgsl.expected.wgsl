fn abs_002533() {
  var arg_0 = vec4<f32>();
  var res : vec4<f32> = abs(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_002533();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  abs_002533();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  abs_002533();
}
