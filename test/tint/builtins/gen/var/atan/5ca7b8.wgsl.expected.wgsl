fn atan_5ca7b8() {
  const arg_0 = vec2(1.0);
  var res = atan(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_5ca7b8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_5ca7b8();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_5ca7b8();
}
