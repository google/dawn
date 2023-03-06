fn quantizeToF16_e8fd14() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = quantizeToF16(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  quantizeToF16_e8fd14();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  quantizeToF16_e8fd14();
}

@compute @workgroup_size(1)
fn compute_main() {
  quantizeToF16_e8fd14();
}
