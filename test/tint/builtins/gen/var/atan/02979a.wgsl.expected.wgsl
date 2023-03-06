fn atan_02979a() {
  var arg_0 = 1.0f;
  var res : f32 = atan(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan_02979a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan_02979a();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan_02979a();
}
