fn step_7c7e5c() {
  var res = step(vec2(1.0), vec2(1.0));
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
