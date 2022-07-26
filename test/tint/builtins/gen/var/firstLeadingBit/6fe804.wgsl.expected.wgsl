fn firstLeadingBit_6fe804() {
  var arg_0 = vec2<u32>(1u);
  var res : vec2<u32> = firstLeadingBit(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstLeadingBit_6fe804();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  firstLeadingBit_6fe804();
}

@compute @workgroup_size(1)
fn compute_main() {
  firstLeadingBit_6fe804();
}
