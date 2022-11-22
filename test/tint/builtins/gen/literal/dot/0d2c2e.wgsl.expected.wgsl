fn dot_0d2c2e() {
  var res = dot(vec2(1.0), vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_0d2c2e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_0d2c2e();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_0d2c2e();
}
