fn abs_8ca9b1() {
  var res = abs(vec4(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  abs_8ca9b1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  abs_8ca9b1();
}

@compute @workgroup_size(1)
fn compute_main() {
  abs_8ca9b1();
}
