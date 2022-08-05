fn frexp_4b2200() {
  var res = frexp(1.0f);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  frexp_4b2200();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  frexp_4b2200();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_4b2200();
}
