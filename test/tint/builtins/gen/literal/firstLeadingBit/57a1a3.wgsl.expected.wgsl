fn firstLeadingBit_57a1a3() {
  var res : i32 = firstLeadingBit(1i);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstLeadingBit_57a1a3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  firstLeadingBit_57a1a3();
}

@compute @workgroup_size(1)
fn compute_main() {
  firstLeadingBit_57a1a3();
}
