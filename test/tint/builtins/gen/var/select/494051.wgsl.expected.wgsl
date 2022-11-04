fn select_494051() {
  const arg_0 = 1.0;
  const arg_1 = 1.0;
  var arg_2 = true;
  var res = select(arg_0, arg_1, arg_2);
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
