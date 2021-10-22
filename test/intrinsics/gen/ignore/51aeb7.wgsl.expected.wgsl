intrinsics/gen/ignore/51aeb7.wgsl:28:3 warning: use of deprecated intrinsic
  ignore(1);
  ^^^^^^

fn ignore_51aeb7() {
  ignore(1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ignore_51aeb7();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ignore_51aeb7();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ignore_51aeb7();
}
