fn select_51b047() {
  var res : vec2<u32> = select(vec2<u32>(1u), vec2<u32>(1u), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_51b047();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_51b047();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_51b047();
}
