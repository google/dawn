fn countLeadingZeros_6d4656() {
  var res : i32 = countLeadingZeros(1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countLeadingZeros_6d4656();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countLeadingZeros_6d4656();
}

@compute @workgroup_size(1)
fn compute_main() {
  countLeadingZeros_6d4656();
}
