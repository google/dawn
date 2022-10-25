fn atanh_70d5bd() {
  const arg_0 = vec2(0.5);
  var res = atanh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_70d5bd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_70d5bd();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_70d5bd();
}
