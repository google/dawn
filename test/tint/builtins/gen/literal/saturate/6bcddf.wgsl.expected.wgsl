fn saturate_6bcddf() {
  var res : vec3<f32> = saturate(vec3<f32>(2.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  saturate_6bcddf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  saturate_6bcddf();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_6bcddf();
}
