fn bitcast_b28cbd() {
  var res : vec3<i32> = bitcast<vec3<i32>>(vec3<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_b28cbd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_b28cbd();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_b28cbd();
}
