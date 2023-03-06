fn sqrt_aa0d7a() {
  var res : vec4<f32> = sqrt(vec4<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sqrt_aa0d7a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sqrt_aa0d7a();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_aa0d7a();
}
