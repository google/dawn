fn clamp_8b1eaa() {
  var res = clamp(vec3(1), vec3(1), vec3(1));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  clamp_8b1eaa();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  clamp_8b1eaa();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_8b1eaa();
}
