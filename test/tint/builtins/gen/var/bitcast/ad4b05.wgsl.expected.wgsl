fn bitcast_ad4b05() {
  var arg_0 = 1u;
  var res : f32 = bitcast<f32>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_ad4b05();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_ad4b05();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_ad4b05();
}
