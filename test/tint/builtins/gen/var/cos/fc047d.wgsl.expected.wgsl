enable f16;

fn cos_fc047d() {
  var arg_0 = f16();
  var res : f16 = cos(arg_0);
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
