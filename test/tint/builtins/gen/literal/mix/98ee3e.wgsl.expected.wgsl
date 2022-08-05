enable f16;

fn mix_98ee3e() {
  var res : vec2<f16> = mix(vec2<f16>(f16()), vec2<f16>(f16()), vec2<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_98ee3e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_98ee3e();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_98ee3e();
}
