fn select_4c4738() {
  var res = select(vec4(1), vec4(1), vec4<bool>(true));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_4c4738();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_4c4738();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_4c4738();
}
