fn countTrailingZeros_d2b4a0() {
  var arg_0 = vec4<u32>(1u);
  var res : vec4<u32> = countTrailingZeros(arg_0);
}

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
