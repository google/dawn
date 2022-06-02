fn firstTrailingBit_3a2acc() {
  var arg_0 = 1;
  var res : i32 = firstTrailingBit(arg_0);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstTrailingBit_3a2acc();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  firstTrailingBit_3a2acc();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  firstTrailingBit_3a2acc();
}
