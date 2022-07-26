fn select_bb447f() {
  var arg_0 = vec2<i32>(1);
  var arg_1 = vec2<i32>(1);
  var arg_2 = true;
  var res : vec2<i32> = select(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_bb447f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_bb447f();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_bb447f();
}
