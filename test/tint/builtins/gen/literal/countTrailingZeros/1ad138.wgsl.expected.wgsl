fn countTrailingZeros_1ad138() {
  var res : vec2<u32> = countTrailingZeros(vec2<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_1ad138();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countTrailingZeros_1ad138();
}

@compute @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_1ad138();
}
