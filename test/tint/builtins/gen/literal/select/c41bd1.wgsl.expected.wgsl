fn select_c41bd1() {
  var res : vec4<bool> = select(vec4<bool>(true), vec4<bool>(true), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_c41bd1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_c41bd1();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_c41bd1();
}
