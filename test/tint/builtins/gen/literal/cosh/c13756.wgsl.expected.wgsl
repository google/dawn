fn cosh_c13756() {
  var res : vec2<f32> = cosh(vec2<f32>(0.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cosh_c13756();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cosh_c13756();
}

@compute @workgroup_size(1)
fn compute_main() {
  cosh_c13756();
}
