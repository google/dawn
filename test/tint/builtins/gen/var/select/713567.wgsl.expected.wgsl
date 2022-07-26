fn select_713567() {
  var arg_0 = vec4<f32>(1.0f);
  var arg_1 = vec4<f32>(1.0f);
  var arg_2 = true;
  var res : vec4<f32> = select(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_713567();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_713567();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_713567();
}
