fn atan2_ae713e() {
  var arg_0 = vec4<f32>(1.0f);
  var arg_1 = vec4<f32>(1.0f);
  var res : vec4<f32> = atan2(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan2_ae713e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan2_ae713e();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan2_ae713e();
}
