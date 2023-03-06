fn firstLeadingBit_a622c2() {
  var res : vec2<i32> = firstLeadingBit(vec2<i32>(1i));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  firstLeadingBit_a622c2();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  firstLeadingBit_a622c2();
}

@compute @workgroup_size(1)
fn compute_main() {
  firstLeadingBit_a622c2();
}
