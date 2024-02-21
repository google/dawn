fn bitcast_a5c539() {
  var res : vec4<i32> = bitcast<vec4<i32>>(vec4<u32>(1u));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_a5c539();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_a5c539();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_a5c539();
}
