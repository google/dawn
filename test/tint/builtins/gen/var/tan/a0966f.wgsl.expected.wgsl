fn tan_a0966f() {
  const arg_0 = vec4(1.0);
  var res = tan(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_a0966f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_a0966f();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_a0966f();
}
