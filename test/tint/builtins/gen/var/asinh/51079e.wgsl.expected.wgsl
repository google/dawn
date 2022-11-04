fn asinh_51079e() {
  const arg_0 = vec3(1.0);
  var res = asinh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_51079e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_51079e();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_51079e();
}
