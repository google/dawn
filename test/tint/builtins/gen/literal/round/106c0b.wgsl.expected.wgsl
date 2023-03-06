fn round_106c0b() {
  var res : vec4<f32> = round(vec4<f32>(3.5f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_106c0b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_106c0b();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_106c0b();
}
