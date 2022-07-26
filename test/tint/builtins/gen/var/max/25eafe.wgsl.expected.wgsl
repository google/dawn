fn max_25eafe() {
  var arg_0 = vec3<i32>(1);
  var arg_1 = vec3<i32>(1);
  var res : vec3<i32> = max(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_25eafe();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_25eafe();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_25eafe();
}
