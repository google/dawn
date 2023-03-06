enable f16;

fn cos_fc047d() {
  var res : f16 = cos(0.0h);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

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
