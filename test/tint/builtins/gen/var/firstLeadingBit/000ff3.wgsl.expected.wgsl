fn firstLeadingBit_000ff3() {
  var arg_0 = vec4<u32>(1u);
  var res : vec4<u32> = firstLeadingBit(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstLeadingBit_000ff3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  firstLeadingBit_000ff3();
}

@compute @workgroup_size(1)
fn compute_main() {
  firstLeadingBit_000ff3();
}
