fn reflect_bba2d0() {
  var res = reflect(vec2(1.0), vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reflect_bba2d0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reflect_bba2d0();
}

@compute @workgroup_size(1)
fn compute_main() {
  reflect_bba2d0();
}
