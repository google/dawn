fn firstTrailingBit_3a2acc() {
  var res : i32 = firstTrailingBit(1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstTrailingBit_3a2acc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  firstTrailingBit_3a2acc();
}

@compute @workgroup_size(1)
fn compute_main() {
  firstTrailingBit_3a2acc();
}
