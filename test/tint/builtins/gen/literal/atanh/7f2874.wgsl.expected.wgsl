fn atanh_7f2874() {
  var res = atanh(vec3(0.5));
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
