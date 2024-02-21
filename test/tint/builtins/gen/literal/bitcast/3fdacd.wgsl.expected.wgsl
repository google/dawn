fn bitcast_3fdacd() {
  var res : vec4<f32> = bitcast<vec4<f32>>(vec4<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_3fdacd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_3fdacd();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_3fdacd();
}
