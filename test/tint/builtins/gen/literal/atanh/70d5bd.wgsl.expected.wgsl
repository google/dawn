fn atanh_70d5bd() {
  var res = atanh(vec2(0.5));
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
