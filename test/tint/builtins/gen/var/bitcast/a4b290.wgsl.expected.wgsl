fn bitcast_a4b290() {
  var arg_0 = vec4<u32>(1u);
  var res : vec4<f32> = bitcast<vec4<f32>>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_a4b290();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_a4b290();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_a4b290();
}
