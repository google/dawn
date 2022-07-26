fn dot_883f0e() {
  var arg_0 = vec2<f32>(1.0f);
  var arg_1 = vec2<f32>(1.0f);
  var res : f32 = dot(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_883f0e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_883f0e();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_883f0e();
}
