intrinsics/gen/ignore/5016e5.wgsl:29:3 warning: use of deprecated intrinsic
  ignore(arg_0);
  ^^^^^^

[[group(1), binding(0)]] var arg_0 : sampler;

fn ignore_5016e5() {
  ignore(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ignore_5016e5();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ignore_5016e5();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ignore_5016e5();
}
