fn extractBits_ce81f8() {
  var res : u32 = extractBits(1u, 1u, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

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
