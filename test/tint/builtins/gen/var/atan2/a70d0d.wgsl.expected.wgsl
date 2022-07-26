fn atan2_a70d0d() {
  var arg_0 = vec3<f32>(1.0f);
  var arg_1 = vec3<f32>(1.0f);
  var res : vec3<f32> = atan2(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan2_a70d0d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan2_a70d0d();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan2_a70d0d();
}
