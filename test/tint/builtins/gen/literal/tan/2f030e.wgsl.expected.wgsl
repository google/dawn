fn tan_2f030e() {
  var res : f32 = tan(1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_2f030e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_2f030e();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_2f030e();
}
