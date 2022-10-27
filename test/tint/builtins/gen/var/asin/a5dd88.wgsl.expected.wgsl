fn asin_a5dd88() {
  const arg_0 = vec2(1);
  var res = asin(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_a5dd88();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_a5dd88();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_a5dd88();
}
