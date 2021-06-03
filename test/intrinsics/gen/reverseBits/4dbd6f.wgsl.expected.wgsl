fn reverseBits_4dbd6f() {
  var res : vec4<i32> = reverseBits(vec4<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  reverseBits_4dbd6f();
}

[[stage(fragment)]]
fn fragment_main() {
  reverseBits_4dbd6f();
}

[[stage(compute)]]
fn compute_main() {
  reverseBits_4dbd6f();
}
