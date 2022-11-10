enable f16;

fn cos_fc047d() {
  var res : f16 = cos(1.0h);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_fc047d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_fc047d();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_fc047d();
}
