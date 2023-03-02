fn asin_64bb1f() {
  const arg_0 = vec4(0.4794255386040000011);
  var res = asin(arg_0);
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
