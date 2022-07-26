fn step_19accd() {
  var arg_0 = vec2<f32>(1.0f);
  var arg_1 = vec2<f32>(1.0f);
  var res : vec2<f32> = step(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  step_19accd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  step_19accd();
}

@compute @workgroup_size(1)
fn compute_main() {
  step_19accd();
}
