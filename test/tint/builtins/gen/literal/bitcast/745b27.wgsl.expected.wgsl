fn bitcast_745b27() {
  var res : vec4<f32> = bitcast<vec4<f32>>(vec4<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_745b27();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_745b27();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_745b27();
}
