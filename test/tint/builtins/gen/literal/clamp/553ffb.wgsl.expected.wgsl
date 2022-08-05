enable f16;

fn clamp_553ffb() {
  var res : f16 = clamp(f16(), f16(), f16());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_553ffb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_553ffb();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_553ffb();
}
