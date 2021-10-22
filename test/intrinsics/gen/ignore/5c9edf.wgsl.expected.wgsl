intrinsics/gen/ignore/5c9edf.wgsl:29:3 warning: use of deprecated intrinsic
  ignore(arg_0);
  ^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_external;

fn ignore_5c9edf() {
  ignore(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ignore_5c9edf();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ignore_5c9edf();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ignore_5c9edf();
}
