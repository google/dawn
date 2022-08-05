enable f16;

fn pow_f37b25() {
  var res : vec2<f16> = pow(vec2<f16>(f16()), vec2<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pow_f37b25();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pow_f37b25();
}

@compute @workgroup_size(1)
fn compute_main() {
  pow_f37b25();
}
