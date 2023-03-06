enable f16;

fn exp_c18fe9() {
  var res : f16 = exp(1.0h);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp_c18fe9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp_c18fe9();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp_c18fe9();
}
