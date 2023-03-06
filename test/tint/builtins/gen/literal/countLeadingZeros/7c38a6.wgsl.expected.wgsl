fn countLeadingZeros_7c38a6() {
  var res : vec3<i32> = countLeadingZeros(vec3<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countLeadingZeros_7c38a6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countLeadingZeros_7c38a6();
}

@compute @workgroup_size(1)
fn compute_main() {
  countLeadingZeros_7c38a6();
}
