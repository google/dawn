fn select_1e960b() {
  var res : vec2<u32> = select(vec2<u32>(1u), vec2<u32>(1u), vec2<bool>(true));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_1e960b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_1e960b();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_1e960b();
}
