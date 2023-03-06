fn asin_7b6a44() {
  var arg_0 = vec2<f32>(0.47942554950714111328f);
  var res : vec2<f32> = asin(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asin_7b6a44();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asin_7b6a44();
}

@compute @workgroup_size(1)
fn compute_main() {
  asin_7b6a44();
}
