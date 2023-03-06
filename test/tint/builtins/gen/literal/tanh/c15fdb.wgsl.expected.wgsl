fn tanh_c15fdb() {
  var res : f32 = tanh(1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tanh_c15fdb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tanh_c15fdb();
}

@compute @workgroup_size(1)
fn compute_main() {
  tanh_c15fdb();
}
