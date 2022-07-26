fn clamp_6c1749() {
  var arg_0 = vec2<i32>(1);
  var arg_1 = vec2<i32>(1);
  var arg_2 = vec2<i32>(1);
  var res : vec2<i32> = clamp(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_6c1749();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_6c1749();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_6c1749();
}
