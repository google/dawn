fn sin_b78c91() {
  var arg_0 = 1.57079637050628662109f;
  var res : f32 = sin(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_b78c91();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_b78c91();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_b78c91();
}
