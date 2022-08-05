enable f16;

fn clamp_b195eb() {
  var res : vec3<f16> = clamp(vec3<f16>(f16()), vec3<f16>(f16()), vec3<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_b195eb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_b195eb();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_b195eb();
}
