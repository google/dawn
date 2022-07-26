fn select_00b848() {
  var res : vec2<i32> = select(vec2<i32>(1), vec2<i32>(1), vec2<bool>(true));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_00b848();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_00b848();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_00b848();
}
