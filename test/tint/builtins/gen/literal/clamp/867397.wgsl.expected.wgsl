fn clamp_867397() {
  var res : vec3<f32> = clamp(vec3<f32>(1.0f), vec3<f32>(1.0f), vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_867397();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_867397();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_867397();
}
