fn select_494051() {
  var res = select(1.0, 1.0, true);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_494051();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_494051();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_494051();
}
