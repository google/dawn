fn extractBits_ce81f8() {
  var res : u32 = extractBits(1u, 1u, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  extractBits_ce81f8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  extractBits_ce81f8();
}

@compute @workgroup_size(1)
fn compute_main() {
  extractBits_ce81f8();
}
