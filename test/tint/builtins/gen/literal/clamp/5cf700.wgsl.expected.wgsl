fn clamp_5cf700() {
  var res = clamp(vec3(1.0), vec3(1.0), vec3(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_5cf700();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_5cf700();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_5cf700();
}
