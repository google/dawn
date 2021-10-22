intrinsics/gen/ignore/2a6ac2.wgsl:29:3 warning: use of deprecated intrinsic
  ignore(arg_0);
  ^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_depth_multisampled_2d;

fn ignore_2a6ac2() {
  ignore(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ignore_2a6ac2();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ignore_2a6ac2();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ignore_2a6ac2();
}
