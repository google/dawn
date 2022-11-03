fn ceil_bb2ca2() {
  const arg_0 = vec2(1.5);
  var res = ceil(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_bb2ca2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ceil_bb2ca2();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_bb2ca2();
}
