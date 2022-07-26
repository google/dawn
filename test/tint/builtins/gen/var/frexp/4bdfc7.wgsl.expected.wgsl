fn frexp_4bdfc7() {
  var arg_0 = vec2<f32>(1.0f);
  var res = frexp(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  frexp_4bdfc7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  frexp_4bdfc7();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_4bdfc7();
}
