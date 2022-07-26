fn reverseBits_c21bc1() {
  var res : vec3<i32> = reverseBits(vec3<i32>(1));
}

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
