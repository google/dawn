fn sinh_b9860e() {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = sinh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sinh_b9860e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sinh_b9860e();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_b9860e();
}
