fn asinh_cf8603() {
  var res = asinh(vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_cf8603();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_cf8603();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_cf8603();
}
