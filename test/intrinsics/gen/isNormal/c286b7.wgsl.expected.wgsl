intrinsics/gen/isNormal/c286b7.wgsl:28:25 warning: use of deprecated intrinsic
  var res: vec3<bool> = isNormal(vec3<f32>());
                        ^^^^^^^^

fn isNormal_c286b7() {
  var res : vec3<bool> = isNormal(vec3<f32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  isNormal_c286b7();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  isNormal_c286b7();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  isNormal_c286b7();
}
