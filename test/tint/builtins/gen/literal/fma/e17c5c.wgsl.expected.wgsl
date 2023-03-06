fn fma_e17c5c() {
  var res : vec3<f32> = fma(vec3<f32>(1.0f), vec3<f32>(1.0f), vec3<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_e17c5c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_e17c5c();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_e17c5c();
}
