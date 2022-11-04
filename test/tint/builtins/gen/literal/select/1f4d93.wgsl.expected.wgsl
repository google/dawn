fn select_1f4d93() {
  var res = select(vec2(1.0), vec2(1.0), vec2<bool>(true));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_1f4d93();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_1f4d93();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_1f4d93();
}
