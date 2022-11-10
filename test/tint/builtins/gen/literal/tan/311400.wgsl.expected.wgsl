fn tan_311400() {
  var res = tan(1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_311400();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_311400();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_311400();
}
