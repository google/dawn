fn tan_2f030e() {
  var arg_0 = 1.0f;
  var res : f32 = tan(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_2f030e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_2f030e();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_2f030e();
}
