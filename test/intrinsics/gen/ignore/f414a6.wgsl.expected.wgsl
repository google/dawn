intrinsics/gen/ignore/f414a6.wgsl:28:3 warning: use of deprecated intrinsic
  ignore(bool());
  ^^^^^^

fn ignore_f414a6() {
  ignore(bool());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ignore_f414a6();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ignore_f414a6();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ignore_f414a6();
}
