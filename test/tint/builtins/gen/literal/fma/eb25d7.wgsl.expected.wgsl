fn fma_eb25d7() {
  var res = fma(vec3(1.0), vec3(1.0), vec3(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_eb25d7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_eb25d7();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_eb25d7();
}
