fn fract_943cb1() {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = fract(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_943cb1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_943cb1();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_943cb1();
}
