fn countLeadingZeros_208d46() {
  var res : u32 = countLeadingZeros(1u);
}

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
