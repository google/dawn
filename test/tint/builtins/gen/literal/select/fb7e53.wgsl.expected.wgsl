fn select_fb7e53() {
  var res : vec2<bool> = select(vec2<bool>(true), vec2<bool>(true), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_fb7e53();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_fb7e53();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_fb7e53();
}
