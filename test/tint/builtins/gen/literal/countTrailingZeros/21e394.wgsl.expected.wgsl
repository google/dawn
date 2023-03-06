fn countTrailingZeros_21e394() {
  var res : u32 = countTrailingZeros(1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_21e394();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countTrailingZeros_21e394();
}

@compute @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_21e394();
}
