fn extractBits_a99a8d() {
  var arg_0 = vec2<i32>(1);
  var arg_1 = 1u;
  var arg_2 = 1u;
  var res : vec2<i32> = extractBits(arg_0, arg_1, arg_2);
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
