fn min_c73147() {
  var arg_0 = 1;
  var arg_1 = 1;
  var res : i32 = min(arg_0, arg_1);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_c73147();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  min_c73147();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  min_c73147();
}
