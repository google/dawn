fn abs_82ff9d() {
  const arg_0 = vec2(1.0);
  var res = abs(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_82ff9d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_82ff9d();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_82ff9d();
}
