fn extractBits_f28f69() {
  var arg_0 = vec2<u32>();
  var arg_1 = 1u;
  var arg_2 = 1u;
  var res : vec2<u32> = extractBits(arg_0, arg_1, arg_2);
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
