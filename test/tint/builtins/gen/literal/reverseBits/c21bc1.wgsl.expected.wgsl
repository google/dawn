fn reverseBits_c21bc1() {
  var res : vec3<i32> = reverseBits(vec3<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reverseBits_c21bc1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reverseBits_c21bc1();
}

@compute @workgroup_size(1)
fn compute_main() {
  reverseBits_c21bc1();
}
