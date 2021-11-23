intrinsics/gen/isNan/4d280d.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec4<bool> = isNan(vec4<f32>());
                        ^^^^^

fn isNan_4d280d() {
  var res : vec4<bool> = isNan(vec4<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isNan_4d280d();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isNan_4d280d();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isNan_4d280d();
}
