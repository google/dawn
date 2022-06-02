fn max_0c0aae() {
  var arg_0 = 1u;
  var arg_1 = 1u;
  var res : u32 = max(arg_0, arg_1);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_0c0aae();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  max_0c0aae();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  max_0c0aae();
}
