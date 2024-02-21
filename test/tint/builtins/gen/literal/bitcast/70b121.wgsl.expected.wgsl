fn bitcast_70b121() {
  var res : vec2<u32> = bitcast<vec2<u32>>(vec2(1));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_70b121();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_70b121();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_70b121();
}
