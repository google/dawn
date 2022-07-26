fn normalize_64d8c0() {
  var res : vec3<f32> = normalize(vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  normalize_64d8c0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  normalize_64d8c0();
}

@compute @workgroup_size(1)
fn compute_main() {
  normalize_64d8c0();
}
