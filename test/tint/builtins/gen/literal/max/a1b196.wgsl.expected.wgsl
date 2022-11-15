fn max_a1b196() {
  var res = max(vec3(1.0), vec3(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_a1b196();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_a1b196();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_a1b196();
}
