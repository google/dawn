fn min_46c5d3() {
  var arg_0 = 1u;
  var arg_1 = 1u;
  var res : u32 = min(arg_0, arg_1);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_46c5d3();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  min_46c5d3();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  min_46c5d3();
}
