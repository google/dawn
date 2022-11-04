fn select_e381c3() {
  var res = select(vec4(1), vec4(1), true);
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
