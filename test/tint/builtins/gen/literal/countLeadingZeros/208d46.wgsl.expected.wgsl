fn countLeadingZeros_208d46() {
  var res : u32 = countLeadingZeros(1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countLeadingZeros_208d46();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countLeadingZeros_208d46();
}

@compute @workgroup_size(1)
fn compute_main() {
  countLeadingZeros_208d46();
}
