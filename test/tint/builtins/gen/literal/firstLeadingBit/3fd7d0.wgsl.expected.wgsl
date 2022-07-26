fn firstLeadingBit_3fd7d0() {
  var res : vec3<u32> = firstLeadingBit(vec3<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstLeadingBit_3fd7d0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  firstLeadingBit_3fd7d0();
}

@compute @workgroup_size(1)
fn compute_main() {
  firstLeadingBit_3fd7d0();
}
