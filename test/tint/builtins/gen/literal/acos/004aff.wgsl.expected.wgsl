enable f16;

fn acos_004aff() {
  var res : vec2<f16> = acos(vec2<f16>(0.96875h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  acos_004aff();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  acos_004aff();
}

@compute @workgroup_size(1)
fn compute_main() {
  acos_004aff();
}
