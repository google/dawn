intrinsics/gen/isNan/1280ab.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec3<bool> = isNan(vec3<f32>());
                        ^^^^^

fn isNan_1280ab() {
  var res : vec3<bool> = isNan(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isNan_1280ab();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isNan_1280ab();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isNan_1280ab();
}
