fn dot_ef6b1d() {
  var arg_0 = vec4<i32>(1);
  var arg_1 = vec4<i32>(1);
  var res : i32 = dot(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_ef6b1d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_ef6b1d();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_ef6b1d();
}
