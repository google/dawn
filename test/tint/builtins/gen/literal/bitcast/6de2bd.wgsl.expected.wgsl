fn bitcast_6de2bd() {
  var res : vec4<i32> = bitcast<vec4<i32>>(vec4<f32>(1.0f));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_6de2bd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_6de2bd();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_6de2bd();
}
