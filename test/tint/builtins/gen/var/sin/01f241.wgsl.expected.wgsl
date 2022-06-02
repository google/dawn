fn sin_01f241() {
  var arg_0 = vec3<f32>();
  var res : vec3<f32> = sin(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_01f241();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  sin_01f241();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  sin_01f241();
}
