fn min_03c7e3() {
  var arg_0 = vec2<i32>(1);
  var arg_1 = vec2<i32>(1);
  var res : vec2<i32> = min(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_03c7e3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_03c7e3();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_03c7e3();
}
