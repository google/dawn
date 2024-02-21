fn bitcast_c69aaf() {
  var arg_0 = vec2<u32>(1u);
  var res : vec2<i32> = bitcast<vec2<i32>>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_c69aaf();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_c69aaf();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_c69aaf();
}
