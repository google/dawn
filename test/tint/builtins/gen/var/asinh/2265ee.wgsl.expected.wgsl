fn asinh_2265ee() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = asinh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_2265ee();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_2265ee();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_2265ee();
}
