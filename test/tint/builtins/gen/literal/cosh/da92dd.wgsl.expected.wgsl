fn cosh_da92dd() {
  var res : f32 = cosh(0.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cosh_da92dd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cosh_da92dd();
}

@compute @workgroup_size(1)
fn compute_main() {
  cosh_da92dd();
}
