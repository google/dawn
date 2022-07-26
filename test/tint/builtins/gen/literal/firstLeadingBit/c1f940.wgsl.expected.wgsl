fn firstLeadingBit_c1f940() {
  var res : vec4<i32> = firstLeadingBit(vec4<i32>(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstLeadingBit_c1f940();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  firstLeadingBit_c1f940();
}

@compute @workgroup_size(1)
fn compute_main() {
  firstLeadingBit_c1f940();
}
