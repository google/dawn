fn asin_a6d73a() {
  const arg_0 = 0.4794255386040000011;
  var res = asin(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_a6d73a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_a6d73a();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_a6d73a();
}
