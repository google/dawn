fn dot_0c577b() {
  var arg_0 = vec4<f32>(1.0f);
  var arg_1 = vec4<f32>(1.0f);
  var res : f32 = dot(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_0c577b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_0c577b();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_0c577b();
}
