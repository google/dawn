enable f16;

fn atan2_d983ab() {
  var res : vec4<f16> = atan2(vec4<f16>(1.0h), vec4<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan2_d983ab();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan2_d983ab();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan2_d983ab();
}
