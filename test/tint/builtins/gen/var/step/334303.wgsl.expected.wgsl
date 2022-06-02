fn step_334303() {
  var arg_0 = vec3<f32>();
  var arg_1 = vec3<f32>();
  var res : vec3<f32> = step(arg_0, arg_1);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  step_334303();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  step_334303();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  step_334303();
}
