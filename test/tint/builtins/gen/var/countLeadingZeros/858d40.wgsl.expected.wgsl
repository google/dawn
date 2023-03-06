fn countLeadingZeros_858d40() {
  var arg_0 = vec2<i32>(1i);
  var res : vec2<i32> = countLeadingZeros(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countLeadingZeros_858d40();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countLeadingZeros_858d40();
}

@compute @workgroup_size(1)
fn compute_main() {
  countLeadingZeros_858d40();
}
