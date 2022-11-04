fn select_3a14be() {
  var res = select(vec2(1), vec2(1), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_3a14be();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_3a14be();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_3a14be();
}
