fn atan2_57fb13() {
  var res : vec2<f32> = atan2(vec2<f32>(1.0f), vec2<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atan2_57fb13();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atan2_57fb13();
}

@compute @workgroup_size(1)
fn compute_main() {
  atan2_57fb13();
}
