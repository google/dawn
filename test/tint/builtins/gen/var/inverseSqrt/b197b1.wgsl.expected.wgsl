fn inverseSqrt_b197b1() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = inverseSqrt(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  inverseSqrt_b197b1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  inverseSqrt_b197b1();
}

@compute @workgroup_size(1)
fn compute_main() {
  inverseSqrt_b197b1();
}
