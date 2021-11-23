intrinsics/gen/isFinite/8a23ad.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec3<bool> = isFinite(vec3<f32>());
                        ^^^^^^^^

fn isFinite_8a23ad() {
  var res : vec3<bool> = isFinite(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isFinite_8a23ad();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isFinite_8a23ad();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isFinite_8a23ad();
}
