fn bitcast_e61c57() {
  var arg_0 = 1u;
  var res : i32 = bitcast<i32>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_e61c57();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_e61c57();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_e61c57();
}
