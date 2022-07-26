fn atan2_57fb13() {
  var arg_0 = vec2<f32>(1.0f);
  var arg_1 = vec2<f32>(1.0f);
  var res : vec2<f32> = atan2(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan2_57fb13();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan2_57fb13();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan2_57fb13();
}
