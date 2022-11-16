enable f16;

fn trunc_cc2b0d() {
  var res : f16 = trunc(1.5h);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  trunc_cc2b0d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  trunc_cc2b0d();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_cc2b0d();
}
