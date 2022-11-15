fn min_364910() {
  var res = min(vec3(1.0), vec3(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_364910();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_364910();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_364910();
}
