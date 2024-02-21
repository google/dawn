fn bitcast_160c09() {
  const arg_0 = vec4(1);
  var res : vec4<u32> = bitcast<vec4<u32>>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_160c09();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_160c09();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_160c09();
}
