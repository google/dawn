fn firstLeadingBit_57a1a3() {
  var arg_0 = 1;
  var res : i32 = firstLeadingBit(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstLeadingBit_57a1a3();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  firstLeadingBit_57a1a3();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  firstLeadingBit_57a1a3();
}
