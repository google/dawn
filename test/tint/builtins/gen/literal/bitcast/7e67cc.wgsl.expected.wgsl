fn bitcast_7e67cc() {
  var res : i32 = bitcast<i32>(1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : i32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_7e67cc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_7e67cc();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_7e67cc();
}
