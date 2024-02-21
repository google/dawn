fn bitcast_ac09d0() {
  var arg_0 = 1.0f;
  var res : f32 = bitcast<f32>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_ac09d0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_ac09d0();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_ac09d0();
}
