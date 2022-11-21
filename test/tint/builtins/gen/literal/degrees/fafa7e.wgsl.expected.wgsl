fn degrees_fafa7e() {
  var res = degrees(1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_fafa7e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  degrees_fafa7e();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_fafa7e();
}
