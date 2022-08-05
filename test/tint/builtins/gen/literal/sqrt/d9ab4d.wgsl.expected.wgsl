enable f16;

fn sqrt_d9ab4d() {
  var res : vec2<f16> = sqrt(vec2<f16>(f16()));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sqrt_d9ab4d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sqrt_d9ab4d();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_d9ab4d();
}
