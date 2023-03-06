fn firstLeadingBit_57a1a3() {
  var arg_0 = 1i;
  var res : i32 = firstLeadingBit(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

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
