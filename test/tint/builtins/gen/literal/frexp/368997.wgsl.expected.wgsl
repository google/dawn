fn frexp_368997() {
  var res = frexp(vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  frexp_368997();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  frexp_368997();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_368997();
}
