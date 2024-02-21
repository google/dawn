fn bitcast_56266e() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<u32> = bitcast<vec3<u32>>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_56266e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_56266e();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_56266e();
}
