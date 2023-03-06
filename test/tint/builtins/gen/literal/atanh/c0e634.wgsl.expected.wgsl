fn atanh_c0e634() {
  var res : vec2<f32> = atanh(vec2<f32>(0.5f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_c0e634();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_c0e634();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_c0e634();
}
