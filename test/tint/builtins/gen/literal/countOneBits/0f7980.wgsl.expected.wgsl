fn countOneBits_0f7980() {
  var res : vec4<i32> = countOneBits(vec4<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countOneBits_0f7980();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countOneBits_0f7980();
}

@compute @workgroup_size(1)
fn compute_main() {
  countOneBits_0f7980();
}
