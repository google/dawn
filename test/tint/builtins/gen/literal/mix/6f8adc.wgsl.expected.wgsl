fn mix_6f8adc() {
  var res : vec2<f32> = mix(vec2<f32>(1.0f), vec2<f32>(1.0f), vec2<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_6f8adc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_6f8adc();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_6f8adc();
}
