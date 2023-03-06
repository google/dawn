fn fract_fa5c71() {
  var arg_0 = 1.25f;
  var res : f32 = fract(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fract_fa5c71();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fract_fa5c71();
}

@compute @workgroup_size(1)
fn compute_main() {
  fract_fa5c71();
}
