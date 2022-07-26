fn tan_244e2a() {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = tan(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  tan_244e2a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  tan_244e2a();
}

@compute @workgroup_size(1)
fn compute_main() {
  tan_244e2a();
}
