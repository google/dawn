fn countLeadingZeros_f70103() {
  var res : vec4<u32> = countLeadingZeros(vec4<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countLeadingZeros_f70103();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countLeadingZeros_f70103();
}

@compute @workgroup_size(1)
fn compute_main() {
  countLeadingZeros_f70103();
}
