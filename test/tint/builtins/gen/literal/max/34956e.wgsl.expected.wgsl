enable f16;

fn max_34956e() {
  var res : vec2<f16> = max(vec2<f16>(f16()), vec2<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_34956e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_34956e();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_34956e();
}
