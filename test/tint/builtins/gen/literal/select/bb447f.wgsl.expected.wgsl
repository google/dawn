fn select_bb447f() {
  var res : vec2<i32> = select(vec2<i32>(1), vec2<i32>(1), true);
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
