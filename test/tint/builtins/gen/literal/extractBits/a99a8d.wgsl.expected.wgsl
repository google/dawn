fn extractBits_a99a8d() {
  var res : vec2<i32> = extractBits(vec2<i32>(1), 1u, 1u);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  extractBits_a99a8d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  extractBits_a99a8d();
}

@compute @workgroup_size(1)
fn compute_main() {
  extractBits_a99a8d();
}
