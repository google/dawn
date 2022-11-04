fn select_9b478d() {
  var res = select(1, 1, true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_9b478d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_9b478d();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_9b478d();
}
