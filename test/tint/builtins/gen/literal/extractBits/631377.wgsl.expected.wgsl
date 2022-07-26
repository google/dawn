fn extractBits_631377() {
  var res : vec4<u32> = extractBits(vec4<u32>(1u), 1u, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  extractBits_631377();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  extractBits_631377();
}

@compute @workgroup_size(1)
fn compute_main() {
  extractBits_631377();
}
