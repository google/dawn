fn sign_dd790e() {
  var res : f32 = sign(1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sign_dd790e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sign_dd790e();
}

@compute @workgroup_size(1)
fn compute_main() {
  sign_dd790e();
}
