fn asin_64bb1f() {
  var res = asin(vec4(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_64bb1f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_64bb1f();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_64bb1f();
}
