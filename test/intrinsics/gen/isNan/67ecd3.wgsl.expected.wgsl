intrinsics/gen/isNan/67ecd3.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec2<bool> = isNan(vec2<f32>());
                        ^^^^^

fn isNan_67ecd3() {
  var res : vec2<bool> = isNan(vec2<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isNan_67ecd3();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isNan_67ecd3();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isNan_67ecd3();
}
