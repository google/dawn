intrinsics/gen/ignore/c8a0ee.wgsl:29:3 warning: use of deprecated intrinsic
  ignore(arg_0);
  ^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_depth_2d_array;

fn ignore_c8a0ee() {
  ignore(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ignore_c8a0ee();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ignore_c8a0ee();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ignore_c8a0ee();
}
