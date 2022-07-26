fn select_ed8a15() {
  var arg_0 = 1;
  var arg_1 = 1;
  var arg_2 = true;
  var res : i32 = select(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_ed8a15();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_ed8a15();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_ed8a15();
}
