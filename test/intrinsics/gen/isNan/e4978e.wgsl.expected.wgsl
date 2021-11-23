intrinsics/gen/isNan/e4978e.wgsl:28:19 warning: use of deprecated intrinsic
  var res: bool = isNan(1.0);
                  ^^^^^

fn isNan_e4978e() {
  var res : bool = isNan(1.0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isNan_e4978e();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isNan_e4978e();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isNan_e4978e();
}
