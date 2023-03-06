fn unpack2x16snorm_b4aea6() {
  var arg_0 = 1u;
  var res : vec2<f32> = unpack2x16snorm(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  unpack2x16snorm_b4aea6();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  unpack2x16snorm_b4aea6();
}

@compute @workgroup_size(1)
fn compute_main() {
  unpack2x16snorm_b4aea6();
}
