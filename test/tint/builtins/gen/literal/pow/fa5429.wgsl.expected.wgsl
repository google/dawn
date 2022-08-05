enable f16;

fn pow_fa5429() {
  var res : vec3<f16> = pow(vec3<f16>(f16()), vec3<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  pow_fa5429();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  pow_fa5429();
}

@compute @workgroup_size(1)
fn compute_main() {
  pow_fa5429();
}
