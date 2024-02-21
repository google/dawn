fn bitcast_2421c8() {
  var arg_0 = vec2<u32>(1u);
  var res : vec2<f32> = bitcast<vec2<f32>>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_2421c8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_2421c8();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_2421c8();
}
