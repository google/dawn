fn select_bf3d29() {
  var res : vec2<f32> = select(vec2<f32>(1.0f), vec2<f32>(1.0f), true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_bf3d29();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_bf3d29();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_bf3d29();
}
