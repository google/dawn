fn normalize_9a0aab() {
  var res : vec4<f32> = normalize(vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  normalize_9a0aab();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  normalize_9a0aab();
}

@compute @workgroup_size(1)
fn compute_main() {
  normalize_9a0aab();
}
