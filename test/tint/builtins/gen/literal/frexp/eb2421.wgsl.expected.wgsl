fn frexp_eb2421() {
  var res = frexp(vec2<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  frexp_eb2421();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  frexp_eb2421();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_eb2421();
}
