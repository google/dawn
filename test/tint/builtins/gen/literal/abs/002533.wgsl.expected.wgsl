fn abs_002533() {
  var res : vec4<f32> = abs(vec4<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_002533();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_002533();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_002533();
}
