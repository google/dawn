fn saturate_e40fb6() {
  var res = saturate(vec2(2.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  saturate_e40fb6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  saturate_e40fb6();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_e40fb6();
}
