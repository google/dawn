fn unpack2x16unorm_7699c0() {
  var arg_0 = 1u;
  var res : vec2<f32> = unpack2x16unorm(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  unpack2x16unorm_7699c0();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  unpack2x16unorm_7699c0();
}

@compute @workgroup_size(1)
fn compute_main() {
  unpack2x16unorm_7699c0();
}
