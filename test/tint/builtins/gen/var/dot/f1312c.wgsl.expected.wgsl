fn dot_f1312c() {
  var arg_0 = vec3<i32>(1);
  var arg_1 = vec3<i32>(1);
  var res : i32 = dot(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  dot_f1312c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  dot_f1312c();
}

@compute @workgroup_size(1)
fn compute_main() {
  dot_f1312c();
}
