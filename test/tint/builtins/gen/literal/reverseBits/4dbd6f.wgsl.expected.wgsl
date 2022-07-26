fn reverseBits_4dbd6f() {
  var res : vec4<i32> = reverseBits(vec4<i32>(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reverseBits_4dbd6f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reverseBits_4dbd6f();
}

@compute @workgroup_size(1)
fn compute_main() {
  reverseBits_4dbd6f();
}
