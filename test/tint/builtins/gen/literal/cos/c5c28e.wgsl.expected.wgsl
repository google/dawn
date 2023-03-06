fn cos_c5c28e() {
  var res : f32 = cos(0.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_c5c28e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_c5c28e();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_c5c28e();
}
