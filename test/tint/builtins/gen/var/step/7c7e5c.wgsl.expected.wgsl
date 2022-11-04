fn step_7c7e5c() {
  const arg_0 = vec2(1.0);
  const arg_1 = vec2(1.0);
  var res = step(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  step_7c7e5c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  step_7c7e5c();
}

@compute @workgroup_size(1)
fn compute_main() {
  step_7c7e5c();
}
