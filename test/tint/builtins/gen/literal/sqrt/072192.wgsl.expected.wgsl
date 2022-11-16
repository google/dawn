fn sqrt_072192() {
  var res = sqrt(vec3(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sqrt_072192();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sqrt_072192();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_072192();
}
