fn round_52c84d() {
  var arg_0 = vec2<f32>(3.5f);
  var res : vec2<f32> = round(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  round_52c84d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  round_52c84d();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_52c84d();
}
