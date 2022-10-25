fn atanh_7f2874() {
  const arg_0 = vec3(0.5);
  var res = atanh(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_7f2874();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_7f2874();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_7f2874();
}
