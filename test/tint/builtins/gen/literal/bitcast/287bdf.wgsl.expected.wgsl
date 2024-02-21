fn bitcast_287bdf() {
  var res : vec3<u32> = bitcast<vec3<u32>>(vec3<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_287bdf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_287bdf();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_287bdf();
}
