fn reflect_b61e10() {
  var res : vec2<f32> = reflect(vec2<f32>(1.0f), vec2<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reflect_b61e10();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reflect_b61e10();
}

@compute @workgroup_size(1)
fn compute_main() {
  reflect_b61e10();
}
