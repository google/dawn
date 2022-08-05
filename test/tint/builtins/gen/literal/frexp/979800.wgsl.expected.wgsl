fn frexp_979800() {
  var res = frexp(vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  frexp_979800();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  frexp_979800();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_979800();
}
