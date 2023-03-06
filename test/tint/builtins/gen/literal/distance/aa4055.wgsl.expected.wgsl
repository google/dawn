fn distance_aa4055() {
  var res : f32 = distance(vec2<f32>(1.0f), vec2<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  distance_aa4055();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  distance_aa4055();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_aa4055();
}
