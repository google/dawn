fn select_4e60da() {
  var res = select(vec2(1.0), vec2(1.0), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_4e60da();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_4e60da();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_4e60da();
}
