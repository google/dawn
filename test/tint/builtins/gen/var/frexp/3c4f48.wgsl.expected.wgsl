fn frexp_3c4f48() {
  var arg_0 = vec4<f32>(1.0f);
  var res = frexp(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  frexp_3c4f48();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  frexp_3c4f48();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_3c4f48();
}
