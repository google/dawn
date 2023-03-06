fn countTrailingZeros_327c37() {
  var res : vec2<i32> = countTrailingZeros(vec2<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  countTrailingZeros_327c37();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  countTrailingZeros_327c37();
}

@compute @workgroup_size(1)
fn compute_main() {
  countTrailingZeros_327c37();
}
