intrinsics/gen/ignore/d91a2f.wgsl:28:3 warning: use of deprecated intrinsic
  ignore(1.0);
  ^^^^^^

fn ignore_d91a2f() {
  ignore(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ignore_d91a2f();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ignore_d91a2f();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  ignore_d91a2f();
}
