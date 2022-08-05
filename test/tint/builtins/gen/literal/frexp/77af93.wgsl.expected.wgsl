fn frexp_77af93() {
  var res = frexp(vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  frexp_77af93();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  frexp_77af93();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_77af93();
}
