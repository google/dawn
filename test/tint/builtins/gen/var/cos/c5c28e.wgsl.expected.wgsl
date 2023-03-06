fn cos_c5c28e() {
  var arg_0 = 0.0f;
  var res : f32 = cos(arg_0);
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
