fn max_e8192f() {
  var arg_0 = vec2<i32>(1);
  var arg_1 = vec2<i32>(1);
  var res : vec2<i32> = max(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_e8192f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_e8192f();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_e8192f();
}
