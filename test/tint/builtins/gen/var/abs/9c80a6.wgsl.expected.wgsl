fn abs_9c80a6() {
  var arg_0 = vec4<i32>();
  var res : vec4<i32> = abs(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_9c80a6();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  abs_9c80a6();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  abs_9c80a6();
}
