fn saturate_270da5() {
  var arg_0 = 2.0f;
  var res : f32 = saturate(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  saturate_270da5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  saturate_270da5();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_270da5();
}
