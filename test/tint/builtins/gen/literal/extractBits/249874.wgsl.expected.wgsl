fn extractBits_249874() {
  var res : i32 = extractBits(1i, 1u, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  extractBits_249874();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  extractBits_249874();
}

@compute @workgroup_size(1)
fn compute_main() {
  extractBits_249874();
}
