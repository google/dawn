fn tan_8ce3e9() {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = tan(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_8ce3e9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_8ce3e9();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_8ce3e9();
}
