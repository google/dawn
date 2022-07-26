fn select_8fa62c() {
  var arg_0 = vec3<i32>(1);
  var arg_1 = vec3<i32>(1);
  var arg_2 = true;
  var res : vec3<i32> = select(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_8fa62c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_8fa62c();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_8fa62c();
}
