fn extractBits_fb850f() {
  var res : vec4<i32> = extractBits(vec4<i32>(), 1u, 1u);
}

@stage(vertex)
fn vertex_main() -> @builtin(position) vec4<f32> {
  extractBits_fb850f();
  return vec4<f32>();
}

@stage(fragment)
fn fragment_main() {
  extractBits_fb850f();
}

@stage(compute) @workgroup_size(1)
fn compute_main() {
  extractBits_fb850f();
}
