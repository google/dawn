fn abs_577d6e() {
  var res = abs(vec2(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_577d6e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_577d6e();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_577d6e();
}
