fn select_431dfb() {
  var res = select(vec2(1), vec2(1), vec2<bool>(true));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_431dfb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_431dfb();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_431dfb();
}
