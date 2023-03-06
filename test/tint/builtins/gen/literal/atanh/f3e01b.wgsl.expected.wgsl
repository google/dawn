fn atanh_f3e01b() {
  var res : vec4<f32> = atanh(vec4<f32>(0.5f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_f3e01b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_f3e01b();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_f3e01b();
}
