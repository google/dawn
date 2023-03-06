fn reverseBits_222177() {
  var arg_0 = vec2<i32>(1i);
  var res : vec2<i32> = reverseBits(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  reverseBits_222177();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  reverseBits_222177();
}

@compute @workgroup_size(1)
fn compute_main() {
  reverseBits_222177();
}
