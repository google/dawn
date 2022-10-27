fn saturate_6bcddf() {
  var arg_0 = vec3<f32>(2.0f);
  var res : vec3<f32> = saturate(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  saturate_6bcddf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  saturate_6bcddf();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_6bcddf();
}
