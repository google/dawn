fn trunc_562d05() {
  var res : vec3<f32> = trunc(vec3<f32>(1.5f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  trunc_562d05();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  trunc_562d05();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_562d05();
}
