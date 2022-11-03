fn ceil_11b1dc() {
  var res = ceil(vec4(1.5));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ceil_11b1dc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ceil_11b1dc();
}

@compute @workgroup_size(1)
fn compute_main() {
  ceil_11b1dc();
}
