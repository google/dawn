fn mix_1faeb1() {
  var res : vec4<f32> = mix(vec4<f32>(1.0f), vec4<f32>(1.0f), 1.0f);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_1faeb1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_1faeb1();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_1faeb1();
}
