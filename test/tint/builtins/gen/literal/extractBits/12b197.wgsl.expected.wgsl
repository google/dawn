fn extractBits_12b197() {
  var res : vec3<u32> = extractBits(vec3<u32>(1u), 1u, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  extractBits_12b197();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  extractBits_12b197();
}

@compute @workgroup_size(1)
fn compute_main() {
  extractBits_12b197();
}
