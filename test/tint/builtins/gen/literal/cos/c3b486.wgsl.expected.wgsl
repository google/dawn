fn cos_c3b486() {
  var res : vec2<f32> = cos(vec2<f32>(0.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cos_c3b486();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cos_c3b486();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_c3b486();
}
