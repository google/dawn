fn saturate_51567f() {
  var res : vec2<f32> = saturate(vec2<f32>(2.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  saturate_51567f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  saturate_51567f();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_51567f();
}
