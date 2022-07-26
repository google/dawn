fn countOneBits_0f7980() {
  var arg_0 = vec4<i32>(1);
  var res : vec4<i32> = countOneBits(arg_0);
}

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
