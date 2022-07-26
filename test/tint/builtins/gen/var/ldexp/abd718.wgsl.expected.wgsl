fn ldexp_abd718() {
  var arg_0 = vec2<f32>(1.0f);
  var arg_1 = vec2<i32>(1);
  var res : vec2<f32> = ldexp(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  ldexp_abd718();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  ldexp_abd718();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_abd718();
}
