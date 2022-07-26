fn extractBits_e04f5d() {
  var res : vec3<i32> = extractBits(vec3<i32>(1), 1u, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  extractBits_e04f5d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  extractBits_e04f5d();
}

@compute @workgroup_size(1)
fn compute_main() {
  extractBits_e04f5d();
}
