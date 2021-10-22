intrinsics/gen/ignore/b469af.wgsl:29:3 warning: use of deprecated intrinsic
  ignore(arg_0);
  ^^^^^^

[[group(1), binding(0)]] var arg_0 : sampler_comparison;

fn ignore_b469af() {
  ignore(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ignore_b469af();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ignore_b469af();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ignore_b469af();
}
