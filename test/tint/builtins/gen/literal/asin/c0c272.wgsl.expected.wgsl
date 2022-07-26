fn asin_c0c272() {
  var res : f32 = asin(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_c0c272();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_c0c272();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_c0c272();
}
