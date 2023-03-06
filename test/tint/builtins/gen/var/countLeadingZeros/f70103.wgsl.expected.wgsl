fn countLeadingZeros_f70103() {
  var arg_0 = vec4<u32>(1u);
  var res : vec4<u32> = countLeadingZeros(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

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
