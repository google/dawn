fn bitcast_8318a8() {
  var res : vec3<u32> = bitcast<vec3<u32>>(vec3<u32>(1u));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_8318a8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_8318a8();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_8318a8();
}
