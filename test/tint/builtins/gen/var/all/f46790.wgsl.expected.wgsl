fn all_f46790() {
  var arg_0 = vec2<bool>();
  var res : bool = all(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  all_f46790();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  all_f46790();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  all_f46790();
}
