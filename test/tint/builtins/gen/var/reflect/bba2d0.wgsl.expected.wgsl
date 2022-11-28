fn reflect_bba2d0() {
  const arg_0 = vec2(1.0);
  const arg_1 = vec2(1.0);
  var res = reflect(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reflect_bba2d0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reflect_bba2d0();
}

@compute @workgroup_size(1)
fn compute_main() {
  reflect_bba2d0();
}
