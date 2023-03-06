fn fract_8bc1e9() {
  var arg_0 = vec4<f32>(1.25f);
  var res : vec4<f32> = fract(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_8bc1e9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_8bc1e9();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_8bc1e9();
}
