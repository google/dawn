fn min_aa28ad() {
  var res : vec2<f32> = min(vec2<f32>(1.0f), vec2<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  min_aa28ad();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  min_aa28ad();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_aa28ad();
}
