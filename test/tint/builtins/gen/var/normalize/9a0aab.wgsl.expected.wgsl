fn normalize_9a0aab() {
  var arg_0 = vec4<f32>();
  var res : vec4<f32> = normalize(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  normalize_9a0aab();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  normalize_9a0aab();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  normalize_9a0aab();
}
