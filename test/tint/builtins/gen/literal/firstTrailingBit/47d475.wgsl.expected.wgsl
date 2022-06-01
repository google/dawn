fn firstTrailingBit_47d475() {
  var res : u32 = firstTrailingBit(1u);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstTrailingBit_47d475();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  firstTrailingBit_47d475();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  firstTrailingBit_47d475();
}
