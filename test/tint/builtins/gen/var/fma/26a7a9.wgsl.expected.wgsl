fn fma_26a7a9() {
  var arg_0 = vec2<f32>(1.0f);
  var arg_1 = vec2<f32>(1.0f);
  var arg_2 = vec2<f32>(1.0f);
  var res : vec2<f32> = fma(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  fma_26a7a9();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  fma_26a7a9();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_26a7a9();
}
