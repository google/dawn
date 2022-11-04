fn select_e381c3() {
  const arg_0 = vec4(1);
  const arg_1 = vec4(1);
  var arg_2 = true;
  var res = select(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_e381c3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_e381c3();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_e381c3();
}
