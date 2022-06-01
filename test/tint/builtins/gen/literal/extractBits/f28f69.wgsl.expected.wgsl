fn extractBits_f28f69() {
  var res : vec2<u32> = extractBits(vec2<u32>(), 1u, 1u);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  extractBits_f28f69();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  extractBits_f28f69();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  extractBits_f28f69();
}
