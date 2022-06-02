fn extractBits_fb850f() {
  var arg_0 = vec4<i32>();
  var arg_1 = 1u;
  var arg_2 = 1u;
  var res : vec4<i32> = extractBits(arg_0, arg_1, arg_2);
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
