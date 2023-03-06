fn countTrailingZeros_d2b4a0() {
  var res : vec4<u32> = countTrailingZeros(vec4<u32>(1u));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_d2b4a0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countTrailingZeros_d2b4a0();
}

@compute @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_d2b4a0();
}
