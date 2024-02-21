fn bitcast_a8c93f() {
  var arg_0 = vec4<i32>(1i);
  var res : vec4<u32> = bitcast<vec4<u32>>(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  bitcast_a8c93f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  bitcast_a8c93f();
}

@compute @workgroup_size(1)
fn compute_main() {
  bitcast_a8c93f();
}
